use anyhow::Context;
use clap::Parser;
use fmi_rs::{
    model_description::{
        fmi2::{Causality, ModelDescription, VariableType},
        peek_fmi_major_version, FMIMajorVersion,
    },
    zip::extract_zip_archive,
};
use std::{error::Error, fs, path::PathBuf};
use tera;

const MODELICA_TEMPLATE: &str = include_str!("../templates/ExternalFMU.mo.tera");

#[derive(Debug, Parser)]
#[command(
    name = "modelica-fmi",
    about = "Render a Modelica file from a Tera template"
)]
struct Cli {
    #[arg(help = "Path to the FMU")]
    input_file: PathBuf,

    #[arg(help = "Path for the rendered Modelica file")]
    output_file: PathBuf,
}

fn main() -> Result<(), Box<dyn Error>> {
    let arguments = Cli::parse();

    let fmu_path = arguments.input_file;
    let output_file = &arguments.output_file;
    let unzipdir = tempfile::tempdir().context("Failed to create temporary FMU directory")?;

    extract_zip_archive(&fmu_path, unzipdir.path()).context("Failed to extract FMU")?;
    let xml_path: PathBuf = unzipdir.path().join("modelDescription.xml");

    let fmi_major_version =
        peek_fmi_major_version(&xml_path).context("Failed to determine FMI version")?;
    if fmi_major_version != FMIMajorVersion::V2 {
        return Err(format!(
            "Expected an FMI 2.0 FMU, found FMI {:?}",
            fmi_major_version
        )
        .into());
    }

    let model_description = ModelDescription::from_path(&xml_path)?;

    let mut context = tera::Context::new();

    context.insert("model_name", &model_description.modelName);
    context.insert("description", "Generated from a Tera template");
    context.insert("parameters", &modelica_parameters(&model_description));

    let modelica = tera::Tera::one_off(MODELICA_TEMPLATE, &context, false)?;
    
    fs::write(&output_file, modelica)?;

    println!("Created {}", output_file.display());

    Ok(())
}

fn modelica_parameters(model_description: &ModelDescription) -> Vec<String> {
    model_description
        .modelVariables
        .iter()
        .filter(|variable| {
            matches!(
                variable.causality,
                Causality::Parameter | Causality::CalculatedParameter
            )
        })
        .map(|variable| {
            let (modelica_type, start) = match &variable.variableType {
                VariableType::Real { start, .. } => {
                    ("FMI2Real", start.as_deref().unwrap_or("0.0"))
                }
                VariableType::Integer { start, .. }
                | VariableType::Enumeration { start, .. } => {
                    ("FMI2Integer", start.as_deref().unwrap_or("0"))
                }
                VariableType::Boolean { start, .. } => {
                    ("FMI2Boolean", start.as_deref().unwrap_or("false"))
                }
                VariableType::String { start, .. } => {
                    ("FMI2String", start.as_deref().unwrap_or("\"\""))
                }
            };

            let description = variable
                .description
                .as_deref()
                .map(|value| format!(" \"{}\"", modelica_string(value)))
                .unwrap_or_default();

            format!(
                "parameter {modelica_type} {} = {start}{description};",
                modelica_identifier(&variable.name)
            )
        })
        .collect()
}

fn modelica_identifier(value: &str) -> String {
    let mut identifier: String = value
        .chars()
        .map(|character| {
            if character.is_ascii_alphanumeric() || character == '_' {
                character
            } else {
                '_'
            }
        })
        .collect();

    if identifier.is_empty() {
        identifier.push('_');
    }
    if identifier
        .chars()
        .next()
        .is_some_and(|character| character.is_ascii_digit())
    {
        identifier.insert(0, '_');
    }
    identifier
}

fn modelica_string(value: &str) -> String {
    value.replace('"', "\\\"").replace('\n', "\\n")
}
