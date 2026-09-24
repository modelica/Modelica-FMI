use anyhow::{anyhow, bail};
use askama::Template;
use fmi_rs::model_description::fmi3::{
    Causality, Dimension, ModelDescription, ModelVariable, VariableType,
};
use std::{
    collections::HashMap,
    fs::{self},
    path::Path,
};

use modelica_fmi::{
    is_modelica_identifier, modelica_identifier, port_annotation, port_label, update_package_order,
};

#[derive(Template)]
#[template(path = "FMI3CS.mo.askama", escape = "none")]
struct ExternalFmuTemplate<'a> {
    annotations: HashMap<String, String>,
    port_labels: Vec<String>,
    version: String,
    unzipdir: String,
    model_identifier: String,
    instantiation_token: String,
    class_name: String,
    description: Option<String>,
    within: String,
    model_description: &'a ModelDescription,
    parameters: Vec<&'a ModelVariable>,
    inputs: Vec<&'a ModelVariable>,
    outputs: Vec<&'a ModelVariable>,
    icon_width: f64,
    icon_height: f64,
}

impl<'a> ExternalFmuTemplate<'a> {
    pub fn annotation(&self, variable_name: &str) -> String {
        self.annotations
            .get(variable_name)
            .cloned()
            .unwrap_or_default()
    }

    pub fn id(&self, variable_name: &str) -> String {
        modelica_identifier(variable_name)
    }

    pub fn extent(&self, variable: &ModelVariable) -> anyhow::Result<Vec<usize>> {
        variable
            .dimensions
            .iter()
            .map(|d| match d {
                Dimension::Fixed { start } => Ok(*start),
                Dimension::Variable { valueReference } => {
                    let dimension_variable = self
                        .model_description
                        .variable_by_value_reference(*valueReference)?;
                    self.start_literal(dimension_variable)?
                        .parse::<usize>()
                        .map_err(anyhow::Error::from)
                }
            })
            .collect::<Result<Vec<usize>, _>>()
    }

    pub fn subscripts(&self, variable: &ModelVariable) -> anyhow::Result<String> {
        let ext = self.extent(variable)?;
        if ext.is_empty() {
            return Ok(String::new());
        }
        Ok(format!(
            "[{}]",
            ext.iter()
                .map(|e| e.to_string())
                .collect::<Vec<String>>()
                .join(",")
        ))
    }

    pub fn size(&self, variable: &ModelVariable) -> anyhow::Result<usize> {
        if variable.dimensions.is_empty() {
            Ok(1)
        } else {
            Ok(self.extent(variable)?.iter().product())
        }
    }

    pub fn getter_prefix(&self, variable: &ModelVariable) -> anyhow::Result<String> {
        match variable.dimensions.len() {
            0 => Ok("scalar(".to_owned()),
            1 => Ok("".to_owned()),
            _ => Err(anyhow!("Max. number of dimensions for outputs is 1")),
        }
    }

    pub fn getter_suffix(&self, variable: &ModelVariable) -> anyhow::Result<String> {
        match variable.dimensions.len() {
            0 => Ok(")".to_owned()),
            1 => Ok("".to_owned()),
            _ => Err(anyhow!("Max. number of dimensions for outputs is 1")),
        }
    }

    pub fn as_vector(&self, variable: &ModelVariable, pre: bool) -> anyhow::Result<String> {
        let mut name = self.id(&variable.name);
        if pre {
            name = format!("pre({name})")
        }
        match variable.dimensions.len() {
            0 => Ok(format!("{{{name}}}")),
            1 => Ok(name.to_string()),
            2 => Ok(format!("matrix2vector({name})")),
            _ => Err(anyhow!("Max. number of dimensions is 2")),
        }
    }

    pub fn start_literal(&self, variable: &ModelVariable) -> anyhow::Result<String> {
        let values = &variable.variableType.start().unwrap_or_default();

        if variable.dimensions.is_empty() {
            return values
                .first()
                .cloned()
                .ok_or(anyhow!("Variable has not start value"));
        }

        let size = self.model_description.initial_size(variable)?;

        Ok(format_modelica_array(values, &size))
    }
}

/// Formats a flat vector of string literals into a nested Modelica array literal.
pub fn format_modelica_array(values: &[String], size: &[usize]) -> String {
    // Scalar
    if size.is_empty() {
        return String::new();
    }

    // Base case: 1D array
    if size.len() == 1 {
        return format!("{{{}}}", values.join(", "));
    }

    // Recursive case: N-D array
    let sub_sizes = size.get(1..).unwrap_or_default();

    // Calculate the total number of scalar elements per element of the outer dimension
    let sub_element_count: usize = sub_sizes.iter().product();

    // Split the flat slice into chunks corresponding to sub-arrays
    let inner_literals: Vec<String> = values
        .chunks(sub_element_count)
        .map(|chunk| format_modelica_array(chunk, sub_sizes))
        .collect();

    format!("{{{}}}", inner_literals.join(", "))
}

pub trait ModelVariableExt {
    fn description_literal(&self) -> String;
}

impl ModelVariableExt for ModelVariable {
    fn description_literal(&self) -> String {
        self.description
            .clone()
            .map(|s| format!(" \"{s}\""))
            .unwrap_or_default()
    }
}

pub fn create_modelica_file(
    xml_path: &Path,
    unzipdir: &str,
    modelica_path: Vec<String>,
    output_file: &Path,
) -> anyhow::Result<()> {
    let output_path = Path::new(output_file);

    let output_file_ext = output_path.extension().and_then(|s| s.to_str());

    if output_file_ext != Some("mo") {
        bail!("Output file must have the extension '.mo'")
    }

    let class_name = output_path
        .file_stem()
        .and_then(|s| s.to_str())
        .ok_or_else(|| {
            anyhow::anyhow!(
                "Output path '{}' does not have a valid file name stem",
                output_path.display()
            )
        })?;

    if !is_modelica_identifier(class_name) {
        bail!("Output file name must be a valid Modelica identifier")
    }

    let model_description = ModelDescription::from_path(xml_path)?;

    let model_identifier = if let Some(co_simulation) = &model_description.coSimulation {
        co_simulation.modelIdentifier.clone()
    } else {
        bail!("The FMU does not support Co-Simulation");
    };

    let parameters: Vec<&ModelVariable> = model_description
        .modelVariables
        .iter()
        .filter(|v| {
            matches!(v.causality, Causality::Parameter)
                && matches!(
                    v.variableType,
                    VariableType::Float64 { .. }
                        | VariableType::Int32 { .. }
                        | VariableType::Boolean { .. }
                        | VariableType::String { .. }
                )
        })
        .collect();

    let inputs: Vec<&ModelVariable> = model_description
        .modelVariables
        .iter()
        .filter(|v| {
            matches!(v.causality, Causality::Input)
                && matches!(
                    v.variableType,
                    VariableType::Float64 { .. }
                        | VariableType::Int32 { .. }
                        | VariableType::Boolean { .. }
                )
        })
        .collect();

    let outputs: Vec<&ModelVariable> = model_description
        .modelVariables
        .iter()
        .filter(|v| {
            matches!(v.causality, Causality::Output)
                && matches!(
                    v.variableType,
                    VariableType::Float64 { .. }
                        | VariableType::Int32 { .. }
                        | VariableType::Boolean { .. }
                )
        })
        .collect();

    let icon_height = (inputs.len().max(outputs.len()) as f64 * 100.).max(200.);
    let icon_width = icon_height;

    let mut annotations = HashMap::new();

    for (i, variable) in inputs.iter().enumerate() {
        let annotation = port_annotation(icon_width, icon_height, inputs.len(), i, true);
        annotations.insert(variable.name.clone(), annotation);
    }

    for (i, variable) in outputs.iter().enumerate() {
        let annotation = port_annotation(icon_width, icon_height, outputs.len(), i, false);
        annotations.insert(variable.name.clone(), annotation);
    }

    let mut port_labels = vec![];

    for (i, variable) in inputs.iter().enumerate() {
        let label = port_label(
            icon_width,
            icon_height,
            inputs.len(),
            i,
            true,
            &variable.name,
        );
        port_labels.push(label);
    }

    for (i, variable) in outputs.iter().enumerate() {
        let label = port_label(
            icon_width,
            icon_height,
            outputs.len(),
            i,
            false,
            &variable.name,
        );
        port_labels.push(label);
    }

    let template = ExternalFmuTemplate {
        annotations,
        port_labels,
        version: env!("CARGO_PKG_VERSION").to_owned(),
        unzipdir: unzipdir.to_owned(),
        model_identifier,
        instantiation_token: model_description.instantiationToken.clone(),
        class_name: class_name.to_owned(),
        description: model_description.description.clone(),
        within: modelica_path.join("."),
        model_description: &model_description,
        parameters,
        inputs,
        outputs,
        icon_width,
        icon_height,
    };

    let modelica = template.render()?;

    fs::write(output_file, modelica)?;

    if !modelica_path.is_empty() {
        update_package_order(output_file)?;
    }

    Ok(())
}
