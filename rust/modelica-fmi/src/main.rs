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

use crate::fmi2::{create_modelica_file};

mod fmi2;

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

fn modelica_within_path(library_root: &Path, model_path: &Path) -> Option<String> {
    let relative = model_path.strip_prefix(library_root.parent()?).ok()?;
    let package_path = relative.parent().unwrap_or_else(|| Path::new(""));

    let parts: Vec<String> = package_path
        .components()
        .filter_map(|component| match component {
            std::path::Component::Normal(part) => Some(part.to_string_lossy().to_string()),
            _ => None,
        })
        .collect();

    Some(parts.join("."))
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
    
    let within = modelica_within_path(&library_root, output_file)
        .unwrap_or_default();

    match fmi_major_version {
        FMIMajorVersion::V2 => create_modelica_file(&xml_path, &hash, &within, output_file)?,
        FMIMajorVersion::V3 => todo!(),
    }
    if fmi_major_version != FMIMajorVersion::V2 {
        bail!("Expected an FMI 2.0 FMU, found FMI {:?}", fmi_major_version);
    }

    // let model_description = ModelDescription::from_path(&xml_path)?;

    // let model_identifier = if let Some(co_simulation) = &model_description.coSimulation {
    //     co_simulation.modelIdentifier.clone()
    // } else {
    //     bail!("The FMU does not support Co-Simulation");
    // };

    // let mut annotations = HashMap::new();

    // let outputs: Vec<&ScalarVariable> = model_description.modelVariables
    //     .iter()
    //     .filter(|v| matches!(v.causality, Causality::Input | Causality::Output))
    //     .collect();

    // let height = 160;
    // // let x0 = -100;
    // // let y0 = -80;
    // let y1 = 80;
    
    // for (i, variable) in outputs.iter().enumerate() {
    //     let x1 = if variable.causality == Causality::Input {
    //         -120
    //     } else {
    //         100
    //     };

    //     let y = if outputs.len() == 1 {
    //         0
    //     } else if outputs.len() == 2 {
    //         -50 + i as i32 * 100
    //     } else {
    //         y1 - i as i32 * (height / (outputs.len() as i32 - 1))
    //     };

    //     let annotation = format!(" annotation(Placement(transformation(extent={{ {{ {}, {} }}, {{ {}, {} }} }}), iconTransformation(extent={{ {{ {}, {} }}, {{ {}, {} }} }})))",
    //         x1, y - 10, x1 + 20, y + 10, x1, y - 10, x1 + 20, y + 10
    //     );

    //     annotations.insert(variable.name.clone(), annotation);
    // }

    // let within = modelica_within_path(&library_root, output_file)
    //     .unwrap_or_default();

    // let template = ExternalFmuTemplate {
    //     annotations,
    //     version: env!("CARGO_PKG_VERSION").to_owned(),
    //     hash: hash[..7].to_owned(),
    //     model_identifier,
    //     instantiation_token: model_description.guid.clone(),
    //     model_name: model_description.modelName.clone(),
    //     description: model_description.description.clone(),
    //     within,
    //     model_description: &model_description,
    // };

    // let modelica = template.render()?;

    // fs::write(output_file, modelica)?;
    // update_package_order(output_file)?;

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

#[cfg(test)]
mod tests {
    use super::modelica_within_path;
    use std::path::Path;

    #[test]
    fn computes_modelica_within_path_for_nested_library_packages() {
        let library_root = Path::new(r"E:\WS\Modelica-FMI");
        let model_path = Path::new(r"E:\WS\Modelica-FMI\FMI\Examples\FMI2\Controller_FMU_2.mo");

        assert_eq!(
            modelica_within_path(library_root, model_path),
            Some("FMI.Examples.FMI2".to_owned())
        );
    }
}
