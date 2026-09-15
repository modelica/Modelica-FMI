use anyhow::{anyhow, bail};
use askama::Template;
use fmi_rs::{model_description::fmi3::{Causality, Dimension, ModelDescription, ModelVariable, VariableType}, sim::fmi3::parse_variable_value};
use std::{
    collections::HashMap,
    fs::{self},
    path::Path,
};

use crate::{modelica_identifier, update_package_order};

#[derive(Template)]
#[template(path = "FMI3CS.mo.askama", escape = "none")]
struct ExternalFmuTemplate<'a> {
    annotations: HashMap<String, String>,
    version: String,
    hash: String,
    model_identifier: String,
    instantiation_token: String,
    model_name: String,
    description: Option<String>,
    within: String,
    model_description: &'a ModelDescription,
}

impl<'a> ExternalFmuTemplate<'a> {
    pub fn parameters(&self) -> impl Iterator<Item = &'a ModelVariable> {
        self.model_description
            .modelVariables
            .iter()
            .filter(|v| v.causality == Causality::Parameter)
    }
    
    pub fn inputs(&self) -> impl Iterator<Item = &'a ModelVariable> {
        self.model_description
            .modelVariables
            .iter()
            .filter(|v| v.causality == Causality::Input)
    }
    
    pub fn outputs(&self) -> impl Iterator<Item = &'a ModelVariable> {
        self.model_description
            .modelVariables
            .iter()
            .filter(|v| v.causality == Causality::Output)
    }
    
    pub fn annotation(&self, variable_name: &str) -> String {
        self.annotations
            .get(variable_name)
            .cloned()
            .unwrap_or_default()
    }
    
    pub fn id(&self, variable_name: &str) -> String {
        modelica_identifier(variable_name)
    }

    pub fn subscripts(&self, variable_name: &str) -> anyhow::Result<String> {
        let variable = self.model_description.variable_by_name(variable_name)?;
        let subs = variable.dimensions.iter().map(
            |d| match d {
                Dimension::Fixed { start } => Ok(start.to_string()),
                Dimension::Variable { valueReference } => {
                    let dimension_variable = self.model_description.variable_by_value_reference(*valueReference)?;
                    self.start_literal(dimension_variable)
                },
        }).collect::<Result<Vec<String>, _>>()?;
        Ok(format!("[{}]", subs.join(",")))
    }

    pub fn start_literal(&self, variable: &ModelVariable) -> anyhow::Result<String> {

        let values: Vec<String> = match &variable.variableType {
            VariableType::Float64 { start, .. } | VariableType::UInt64 { start, .. } => {
                start.clone().ok_or(anyhow!("D'oh!"))?.split_whitespace().map(|s| s.to_owned()).collect()
            },
            _ => todo!("Not implemented for type {}", variable.variableType.name()),
        };

        if variable.dimensions.is_empty() {
            return values.iter().next().cloned().ok_or(anyhow!("Variable has not start value"))
        }

        let sizes = variable.dimensions.iter().map(
            |d| match d {
                Dimension::Fixed { start } => Ok(*start),
                Dimension::Variable { valueReference } => {
                    let dimension_variable = self.model_description.variable_by_value_reference(*valueReference)?;
                    let size: usize = match &dimension_variable.variableType {
                        VariableType::UInt64 { start, ..} => start.clone().ok_or(anyhow!("noo"))?.parse()?,
                        _ => bail!("noooo"),
                    };
                    Ok(size)
                }
            }).collect::<Result<Vec<usize>, _>>()?;

        let literal = format_modelica_array(&values, &sizes);

        Ok(literal)
    }
}

/// Formats a flat vector of string literals into a nested Modelica array literal.
pub fn format_modelica_array(values: &[String], sizes: &[usize]) -> String {
    if sizes.is_empty() {
        return String::new();
    }

    // Base case: 1D array
    if sizes.len() == 1 {
        return format!("{{{}}}", values.join(", "));
    }

    // Recursive case: N-D array
    let outer_dim = sizes[0];
    let sub_sizes = &sizes[1..];
    
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
    // fn start_literal(&self) -> anyhow::Result<String>;
    fn description_literal(&self) -> String;
}

impl ModelVariableExt for ModelVariable {
    // fn start_literal(&self) -> anyhow::Result<String> {

    //     todo!()
    //     // let value_literals: Vec<String> = match &self.variableType {
    //     //     VariableType::Float64 { start, .. } => {
    //     //         start.clone().ok_or(anyhow!("D'oh!"))?.split_whitespace().map(|s| s.to_owned()).collect()
    //     //     },
    //     //     _ => todo!("Not implemented for type {}", self.variableType.name()),
    //     // };

    //     // let sizes = variable.dimensions.iter().map(
    //     //     |d| match d {
    //     //         Dimension::Fixed { start } => Ok(start.to_string()),
    //     //         Dimension::Variable { valueReference } => self.model_description.variable_by_value_reference(*valueReference)?.start_literal(),
    //     // }).collect::<Result<Vec<usize>, _>>()?;

    //     // for dimension in self.dimensions {

    //     // }

    //     // match &self.variableType {
    //     //     VariableType::Float64 { start, .. } => {

    //     //         let s = start.clone().ok_or(anyhow!("D'oh!"))?;                
    //     //         let literal = match &self.dimensions.len() {
    //     //         0 => s,
    //     //         1 => {
    //     //             let segments: Vec<String> = s.split_whitespace().map(|s| s.to_owned()).collect();
    //     //             format!("{{{}}}", segments.join(", "))
    //     //         },
    //     //         2 => {
    //     //             let segments: Vec<String> = s.split_whitespace().map(|s| s.to_owned()).collect();
    //     //             let rows: Vec<String> = segments.chunks(3).map(|c| c.join(", ")).collect();
    //     //             format!("[{}]", rows.join("; "))
    //     //         },
    //     //         _ => todo!()
    //     //         };

    //     //         Some(literal)
    //     //     },
    //     //     VariableType::UInt64 { start, .. } => start.clone(),
    //     //     // VariableType::Boolean { start, .. } => {
    //     //     //     start.clone().map(
    //     //     //         |s| match s.as_str() {
    //     //     //             "true" | "1" => "true".to_string(),
    //     //     //             _ => "false".to_string(),
    //     //     //         }
    //     //     //     )
    //     //     // },
    //     //     // VariableType::String { start, .. } => start.clone().map(|s | format!("\"{s}\"")),
    //     //     // VariableType::Enumeration { start, .. } => start.clone(),
    //     //     _ => todo!("Not implemented for type {}", self.variableType.name()),
    //     // }.ok_or(anyhow!("Variable {} is missing start attribute.", self.name))
    // }

    fn description_literal(&self) -> String {
        self.description
            .clone()
            .map(|s| format!(" \"{s}\""))
            .unwrap_or_default()
    }
}

pub fn create_modelica_file(
    xml_path: &Path,
    hash: &str,
    within: &str,
    output_file: &Path,
) -> anyhow::Result<()> {
    let model_description = ModelDescription::from_path(xml_path)?;

    let model_identifier = if let Some(co_simulation) = &model_description.coSimulation {
        co_simulation.modelIdentifier.clone()
    } else {
        bail!("The FMU does not support Co-Simulation");
    };

    let mut annotations = HashMap::new();

    let outputs: Vec<&ModelVariable> = model_description
        .modelVariables
        .iter()
        .filter(|v| matches!(v.causality, Causality::Input | Causality::Output))
        .collect();

    let height = 160;
    // let x0 = -100;
    // let y0 = -80;
    let y1 = 80;

    for (i, variable) in outputs.iter().enumerate() {
        let x1 = if variable.causality == Causality::Input {
            -120
        } else {
            100
        };

        let y = if outputs.len() == 1 {
            0
        } else if outputs.len() == 2 {
            -50 + i as i32 * 100
        } else {
            y1 - i as i32 * (height / (outputs.len() as i32 - 1))
        };

        let annotation = format!(
            " annotation(Placement(transformation(extent={{ {{ {}, {} }}, {{ {}, {} }} }}), iconTransformation(extent={{ {{ {}, {} }}, {{ {}, {} }} }})))",
            x1,
            y - 10,
            x1 + 20,
            y + 10,
            x1,
            y - 10,
            x1 + 20,
            y + 10
        );

        annotations.insert(variable.name.clone(), annotation);
    }

    let template = ExternalFmuTemplate {
        annotations,
        version: env!("CARGO_PKG_VERSION").to_owned(),
        hash: hash[..7].to_owned(),
        model_identifier,
        instantiation_token: model_description.instantiationToken.clone(),
        model_name: model_description.modelName.clone(),
        description: model_description.description.clone(),
        within: within.to_owned(),
        model_description: &model_description,
    };

    let modelica = template.render()?;

    fs::write(output_file, modelica)?;

    update_package_order(output_file)?;

    Ok(())
}
