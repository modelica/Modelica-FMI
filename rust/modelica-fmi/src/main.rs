use anyhow::{Context, anyhow, bail};
use clap::Parser;
use fmi_rs::{
    model_description::{FMIMajorVersion, peek_fmi_major_version},
    zip::extract_zip_archive,
};
use sha2::{Digest, Sha256};
use std::{
    fs::{self, File},
    io::{self, Read},
    path::{Path, PathBuf},
};

mod fmi2;
mod fmi3;

#[derive(Debug, Parser)]
#[command(name = "modelica-fmi", about = "Import an FMU into a Modelica library")]
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

fn modelica_within_path(model_path: &Path) -> anyhow::Result<String> {

    let model_parent = model_path.parent().ok_or(anyhow!("d'oh!"))?;

    let model_parent = fs::canonicalize(model_parent)?;

    let mut segments = vec![];

    let mut package_mo = model_parent.join("package.mo");

    while package_mo.is_file() {
        let package_name = model_parent.file_name().ok_or(anyhow!("d'oh!"))?;
        segments.push(package_name.to_str().ok_or(anyhow!("d'oh!"))?.to_owned());
        package_mo = model_parent.parent().ok_or(anyhow!("d'oh!"))?.join("package.mo").to_path_buf();
    }

    Ok(segments.join("."))
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

    let within = modelica_within_path(output_file).unwrap_or_default();

    match fmi_major_version {
        FMIMajorVersion::V2 => fmi2::create_modelica_file(&xml_path, &hash, &within, output_file)?,
        FMIMajorVersion::V3 => fmi3::create_modelica_file(&xml_path, &hash, &within, output_file)?,
    }

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

// fn modelica_string(value: &str) -> String {
//     value.replace('"', "\\\"").replace('\n', "\\n")
// }

#[cfg(test)]
mod tests {
    use super::modelica_within_path;
    use std::path::Path;

    #[test]
    fn computes_modelica_within_path_for_nested_library_packages() {
        let model_path = Path::new(r"E:\WS\Modelica-FMI\FMI\Examples\FMI2\Controller_FMU_2.mo");
        
        assert_eq!(
            modelica_within_path(model_path).unwrap(),
            "FMI.Examples.FMI2".to_owned()
        );
    }

    #[test]
    fn computes_modelica_within_path_standalone_model() {
        let model_path = Path::new(r"C:\Users\tsr2\Documents\Dymola\Controller_FMU_2.mo");

        assert_eq!(
            modelica_within_path(model_path).unwrap(),
            String::new()
        );
    }
}
