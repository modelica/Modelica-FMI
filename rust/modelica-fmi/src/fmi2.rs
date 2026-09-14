use anyhow::{Context, bail};
use askama::Template;
use clap::Parser;
use fmi_rs::{
    model_description::{
        FMIMajorVersion, fmi2::{Causality, ModelDescription, ScalarVariable, VariableType}, peek_fmi_major_version,
    }, zip::extract_zip_archive,
};
use sha2::{Digest, Sha256};
use std::{
    collections::HashMap, fs::{self, File}, io::{self, Read}, path::{Path, PathBuf},
};

use crate::{modelica_identifier, modelica_within_path, update_package_order};

#[derive(Template)]
#[template(path = "FMU2CS.mo.askama", escape = "none")]
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
    pub fn parameters(&self) -> impl Iterator<Item = &'a ScalarVariable> {
        self.model_description
            .modelVariables
            .iter()
            .filter(|v| v.causality == Causality::Parameter)
    }
    pub fn inputs(&self) -> impl Iterator<Item = &'a ScalarVariable> {
        self.model_description
            .modelVariables
            .iter()
            .filter(|v| v.causality == Causality::Input)
    }
    pub fn outputs(&self) -> impl Iterator<Item = &'a ScalarVariable> {
        self.model_description
            .modelVariables
            .iter()
            .filter(|v| v.causality == Causality::Output)
    }
    pub fn annotation(&self, variable_name: &str) -> String {
        self.annotations.get(variable_name).cloned().unwrap_or_default()
    }
    pub fn id(&self, variable_name: &str) -> String {
        modelica_identifier(variable_name)
    }
}

pub struct ModelicaFormatter;

impl ModelicaFormatter {
    pub fn annotation() -> String {
          "annotation(Placement(transformation(extent={ { 100, -60 }, { 120, -40 } }), iconTransformation(extent={ { 100, -60 }, { 120, -40 } })))".to_owned()
    }
}

pub trait ScalarVariableExt {
    fn start_literal(&self) -> String;
    fn description_literal(&self) -> String;
}

impl ScalarVariableExt for ScalarVariable {
    fn start_literal(&self) -> String {
        match &self.variableType {
            VariableType::Real {start, ..} => start.clone().unwrap_or_else(|| "0.0".to_owned()),
        //     VariableType::Integer(start, ..) => start,
        //     VariableType::Boolean(start, ..) => format!("{}", if *start { "true" } else { "false" }),
        //     VariableType::String(start, ..) => format!({"start"}),
        //     VariableType::Enumeration(start, ..) => format!({"start"}),
            _ => "tata".to_owned()
        }
    }

    fn description_literal(&self) -> String {
        self.description.clone().map(|s| format!(" \"{s}\"")).unwrap_or_default()
    }
}

pub fn create_modelica_file(xml_path: &Path, hash: &str, within: &str, output_file: &Path) -> anyhow::Result<()> {
    let model_description = ModelDescription::from_path(&xml_path)?;

    let model_identifier = if let Some(co_simulation) = &model_description.coSimulation {
        co_simulation.modelIdentifier.clone()
    } else {
        bail!("The FMU does not support Co-Simulation");
    };

    let mut annotations = HashMap::new();

    let outputs: Vec<&ScalarVariable> = model_description.modelVariables
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

        let annotation = format!(" annotation(Placement(transformation(extent={{ {{ {}, {} }}, {{ {}, {} }} }}), iconTransformation(extent={{ {{ {}, {} }}, {{ {}, {} }} }})))",
            x1, y - 10, x1 + 20, y + 10, x1, y - 10, x1 + 20, y + 10
        );

        annotations.insert(variable.name.clone(), annotation);
    }

    let template = ExternalFmuTemplate {
        annotations,
        version: env!("CARGO_PKG_VERSION").to_owned(),
        hash: hash[..7].to_owned(),
        model_identifier,
        instantiation_token: model_description.guid.clone(),
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