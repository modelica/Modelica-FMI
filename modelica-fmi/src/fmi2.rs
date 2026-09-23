use anyhow::{anyhow, bail};
use askama::Template;
use fmi_rs::model_description::fmi2::{Causality, ModelDescription, ScalarVariable, VariableType};
use modelica_fmi::{
    is_modelica_identifier, modelica_identifier, port_annotation, port_label, update_package_order,
};
use std::{
    collections::HashMap,
    fs::{self},
    path::Path,
};

#[derive(Template)]
#[template(path = "FMI2CS.mo.askama", escape = "none")]
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
    parameters: Vec<&'a ScalarVariable>,
    inputs: Vec<&'a ScalarVariable>,
    outputs: Vec<&'a ScalarVariable>,
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
}

pub trait ScalarVariableExt {
    fn start_literal(&self) -> anyhow::Result<String>;
    fn description_literal(&self) -> String;
}

impl ScalarVariableExt for ScalarVariable {
    fn start_literal(&self) -> anyhow::Result<String> {
        match &self.variableType {
            VariableType::Real { start, .. } => start.clone(),
            VariableType::Integer { start, .. } => start.clone(),
            VariableType::Boolean { start, .. } => start.clone().map(|s| match s.as_str() {
                "true" | "1" => "true".to_string(),
                _ => "false".to_string(),
            }),
            VariableType::String { start, .. } => start.clone().map(|s| format!("\"{s}\"")),
            VariableType::Enumeration { start, .. } => start.clone(),
        }
        .ok_or(anyhow!(
            "Variable {} is missing start attribute.",
            self.name
        ))
    }

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

    let parameters: Vec<&ScalarVariable> = model_description
        .modelVariables
        .iter()
        .filter(|v| {
            matches!(v.causality, Causality::Parameter)
                && matches!(
                    v.variableType,
                    VariableType::Real { .. }
                        | VariableType::Integer { .. }
                        | VariableType::Boolean { .. }
                        | VariableType::String { .. }
                )
        })
        .collect();

    let inputs: Vec<&ScalarVariable> = model_description
        .modelVariables
        .iter()
        .filter(|v| {
            matches!(v.causality, Causality::Input)
                && matches!(
                    v.variableType,
                    VariableType::Real { .. }
                        | VariableType::Integer { .. }
                        | VariableType::Boolean { .. }
                )
        })
        .collect();

    let outputs: Vec<&ScalarVariable> = model_description
        .modelVariables
        .iter()
        .filter(|v| {
            matches!(v.causality, Causality::Output)
                && matches!(
                    v.variableType,
                    VariableType::Real { .. }
                        | VariableType::Integer { .. }
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
        instantiation_token: model_description.guid.clone(),
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
