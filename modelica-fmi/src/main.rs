use anyhow::{Context, bail};
use clap::Parser;
use fmi_rs::{
    model_description::{FMIMajorVersion, peek_fmi_major_version},
    zip::extract_zip_archive,
};
use modelica_fmi::{modelica_path, sha256_file};
use std::{
    fs::{self},
    path::{Path, PathBuf},
};

mod fmi2;
mod fmi3;

#[derive(Debug, Parser)]
#[command(
    name = "modelica-fmi",
    version,
    propagate_version = true,
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
    let parent = path.parent();
    let start = if path.is_file() { parent? } else { path };

    start
        .ancestors()
        .filter(|directory| directory.join("package.mo").is_file())
        .last()
        .map(Path::to_path_buf)
        .or_else(|| parent.map(Path::to_path_buf))
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

    let unzipdir_name = hash
        .get(0..7)
        .context(format!("Illegal sha256 hash: {hash}"))?;

    let unzipdir = library_root
        .join("Resources")
        .join("FMUs")
        .join(unzipdir_name);

    if unzipdir.exists() {
        if overwrite {
            if verbose {
                println!("Removing FMU directory {}", unzipdir.display());
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

    let modelica_path = modelica_path(output_file).unwrap_or_default();

    let library_name = if let Some(first) = modelica_path.first() {
        first
    } else {
        output_file
            .file_stem()
            .and_then(|s| s.to_str())
            .ok_or(anyhow::anyhow!("Missing file name"))?
    };

    let resource_path = format!("modelica://{library_name}/Resources/FMUs/{unzipdir_name}");

    match fmi_major_version {
        FMIMajorVersion::V2 => {
            fmi2::create_modelica_file(&xml_path, &resource_path, modelica_path, output_file)?
        }
        FMIMajorVersion::V3 => {
            fmi3::create_modelica_file(&xml_path, &resource_path, modelica_path, output_file)?
        }
    }

    if verbose {
        println!("Created {}", output_file.display());
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use modelica_fmi::is_modelica_identifier;

    #[test]
    fn validates_modelica_identifiers() {
        for value in ["_", "Model", "model_2", "a_b"] {
            assert!(is_modelica_identifier(value));
        }

        for value in ["", "2model", "model-name", "model.name", "é"] {
            assert!(!is_modelica_identifier(value));
        }
    }
}

// fn modelica_string(value: &str) -> String {
//     value.replace('"', "\\\"").replace('\n', "\\n")
// }
