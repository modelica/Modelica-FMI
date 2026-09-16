use crate::zip::extract_zip_archive;
use std::path::{Path, PathBuf};

pub fn download_file<P: AsRef<Path>>(url: &str, target_path: P) -> anyhow::Result<()> {
    use std::fs::File;

    let path = target_path.as_ref();

    if let Some(parent) = path.parent() {
        std::fs::create_dir_all(parent)?;
    }

    let mut response = reqwest::blocking::get(url)?;

    if !response.status().is_success() {
        return Err(anyhow::anyhow!(
            "Server returned an error: {}",
            response.status()
        ));
    }

    let mut destination = File::create(path)?;

    std::io::copy(&mut response, &mut destination)?;

    Ok(())
}

pub fn download_reference_fmus<P: AsRef<Path>>(target_path: P) -> anyhow::Result<()> {
    let url =
        "https://github.com/modelica/Reference-FMUs/releases/latest/download/Reference-FMUs.zip";
    let resources_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("tests/resources");
    let archive_path = resources_dir.join("Reference-FMUs.zip");
    download_file(url, &archive_path)?;
    Ok(extract_zip_archive(archive_path, target_path)?)
}
