use anyhow::Context;
use anyhow::bail;
use clap::Parser;
use fmi_rs::{
    model_description::{
        FMIMajorVersion,
        fmi2::{Causality, ModelDescription, VariableType},
        peek_fmi_major_version,
    },
    zip::extract_zip_archive,
};
use sha2::{Digest, Sha256};
use std::{
    fs::{self, File},
    io::{self, Read},
    path::{Path, PathBuf},
};

const MODELICA_TEMPLATE: &str = include_str!("../templates/ExternalFMU.mo.tera");

#[derive(Debug, Parser)]
#[command(
    name = "modelica-fmi",
    about = "Import an FMU into a Modelica library"
)]
struct Cli {
    #[arg(help = "Path to the FMU to import")]
    fmu_file: PathBuf,

    #[arg(help = "Path to the Modelica file to create")]
    modelica_file: PathBuf,

    #[arg(short, long, help = "Enable progress messages")]
    verbose: bool,

    #[arg(
        long,
        help = "Overwrite an existing extraction directory or Modelica file"
    )]
    overwrite: bool,
}

fn get_library_root<P: AsRef<Path>>(path: P) -> Option<PathBuf> {
    let path = path.as_ref();
    let start = if path.is_file() { path.parent()? } else { path };

    start
        .ancestors()
        .filter(|directory| directory.join("package.mo").is_file())
        .last()
        .map(Path::to_path_buf)
}

fn main() -> anyhow::Result<()> {
    let arguments = Cli::parse();

    let fmu_path = arguments.fmu_file;
    let output_file = &arguments.modelica_file;
    let verbose = arguments.verbose;
    let overwrite = arguments.overwrite;

    let library_root =
        get_library_root(output_file).ok_or(anyhow::anyhow!("Failed to find library root"))?;

    let hash = sha256_file(&fmu_path).context(format!("Failed to read {}", fmu_path.display()))?;

    let unzipdir = library_root.join("Resources").join("FMUs").join(&hash[..7]);

    if unzipdir.exists() {
        if overwrite {
            if verbose {
                println!("Removing FMU directory {}", output_file.display());
            }
            fs::remove_dir_all(&unzipdir)
                .with_context(|| format!("Failed to remove existing FMU directory {unzipdir:?}"))?;
        } else {
            bail!(
                "FMU directory {} already exists (use --overwrite)",
                unzipdir.display()
            )
        }
    }

    if verbose {
        println!("Extracting FMU to {}", unzipdir.display());
    }

    extract_zip_archive(&fmu_path, &unzipdir).context("Failed to extract FMU")?;

    if !overwrite && output_file.exists() {
        bail!(
            "Modelica file {} already exists (use --overwrite)",
            output_file.display()
        );
    }

    let xml_path: PathBuf = unzipdir.join("modelDescription.xml");

    let fmi_major_version =
        peek_fmi_major_version(&xml_path).context("Failed to determine FMI version")?;
    if fmi_major_version != FMIMajorVersion::V2 {
        bail!("Expected an FMI 2.0 FMU, found FMI {:?}", fmi_major_version);
    }

    let model_description = ModelDescription::from_path(&xml_path)?;

    let mut context = tera::Context::new();

    context.insert("model_name", &model_description.modelName);
    context.insert("description", "Generated from a Tera template");
    context.insert("parameters", &modelica_parameters(&model_description));
    context.insert("outputs", &modelica_outputs(&model_description));
    context.insert(
        "output_variables",
        &modelica_output_variables(&model_description),
    );
    context.insert(
        "output_updates",
        &modelica_output_updates(&model_description),
    );

    let modelica = tera::Tera::one_off(MODELICA_TEMPLATE, &context, false)?;

    fs::write(output_file, modelica)?;
    update_package_order(output_file)?;

    if verbose {
        println!("Created {}", output_file.display());
    }

    Ok(())
}

fn update_package_order(output_file: &Path) -> io::Result<()> {
    let package_order = output_file
        .parent()
        .unwrap_or_else(|| Path::new("."))
        .join("package.order");

    let entry = output_file
        .file_stem()
        .and_then(|stem| stem.to_str())
        .ok_or_else(|| io::Error::new(io::ErrorKind::InvalidInput, "Invalid Modelica filename"))?;

    let mut contents = match fs::read_to_string(&package_order) {
        Ok(contents) => contents,
        Err(error) if error.kind() == io::ErrorKind::NotFound => String::new(),
        Err(error) => return Err(error),
    };

    if contents.lines().any(|line| line.trim() == entry) {
        return Ok(());
    }

    if !contents.is_empty() && !contents.ends_with('\n') {
        contents.push('\n');
    }

    contents.push_str(entry);
    contents.push('\n');

    fs::write(package_order, contents)
}

fn sha256_file(path: &Path) -> io::Result<String> {
    let mut file = File::open(path)?;
    let mut hasher = Sha256::new();
    let mut buffer = [0_u8; 64 * 1024];

    loop {
        let bytes_read = file.read(&mut buffer)?;
        if bytes_read == 0 {
            break;
        }
        hasher.update(&buffer[..bytes_read]);
    }

    Ok(format!("{:x}", hasher.finalize()))
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
                VariableType::Real { start, .. } => ("FMI2Real", start.as_deref().unwrap_or("0.0")),
                VariableType::Integer { start, .. } | VariableType::Enumeration { start, .. } => {
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

fn modelica_outputs(model_description: &ModelDescription) -> Vec<String> {
    model_description
        .modelVariables
        .iter()
        .filter(|variable| variable.causality == Causality::Output)
        .map(|variable| {
            let modelica_type = match &variable.variableType {
                VariableType::Real { .. } => "FMI2RealOutput",
                VariableType::Integer { .. } | VariableType::Enumeration { .. } => {
                    "FMI2IntegerOutput"
                }
                VariableType::Boolean { .. } => "FMI2BooleanOutput",
                VariableType::String { .. } => "FMI2StringOutput",
            };

            let attributes = match &variable.variableType {
                VariableType::Real { unit, quantity, .. } => {
                    let mut attributes = Vec::new();
                    if let Some(unit) = unit {
                        attributes.push(format!("unit=\"{}\"", modelica_string(unit)));
                    }
                    if let Some(quantity) = quantity {
                        attributes.push(format!("quantity=\"{}\"", modelica_string(quantity)));
                    }
                    if attributes.is_empty() {
                        String::new()
                    } else {
                        format!("({})", attributes.join(", "))
                    }
                }
                _ => String::new(),
            };

            let description = variable
                .description
                .as_deref()
                .map(|value| format!(" \"{}\"", modelica_string(value)))
                .unwrap_or_default();

            format!(
                "{modelica_type} {}{attributes}{description};",
                modelica_identifier(&variable.name)
            )
        })
        .collect()
}

fn modelica_output_variables(model_description: &ModelDescription) -> Vec<String> {
    model_description
        .modelVariables
        .iter()
        .filter(|variable| variable.causality == Causality::Output)
        .map(|variable| {
            let modelica_type = match &variable.variableType {
                VariableType::Real { .. } => "Real",
                VariableType::Integer { .. } | VariableType::Enumeration { .. } => "Integer",
                VariableType::Boolean { .. } => "Boolean",
                VariableType::String { .. } => "String",
            };

            format!("{modelica_type} {};", modelica_identifier(&variable.name))
        })
        .collect()
}

fn modelica_output_updates(model_description: &ModelDescription) -> Vec<String> {
    model_description
        .modelVariables
        .iter()
        .filter(|variable| variable.causality == Causality::Output)
        .map(|variable| {
            let (getter, argument) = match &variable.variableType {
                VariableType::Real { .. } => ("FMI2GetReal", "valueReference"),
                VariableType::Integer { .. } | VariableType::Enumeration { .. } => {
                    ("FMI2GetInteger", "valueReference")
                }
                VariableType::Boolean { .. } => ("FMI2GetBoolean", "valueReference"),
                VariableType::String { .. } => ("FMI2GetString", "valueReference"),
            };

            format!(
                "    outputVariables.{} := {getter}(instance, {argument}={});",
                modelica_identifier(&variable.name),
                variable.valueReference
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
