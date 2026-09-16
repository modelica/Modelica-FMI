use anyhow::anyhow;
use std::{fs, path::Path};

pub fn modelica_path(model_path: &Path) -> anyhow::Result<Vec<String>> {
    let model_parent = model_path.parent().ok_or_else(|| {
        anyhow!("Model path '{}' has no parent directory", model_path.display())
    })?;

    let mut parent_dir = fs::canonicalize(model_parent).map_err(|error| {
        anyhow!(
            "Failed to resolve model parent directory '{}': {error}",
            model_parent.display()
        )
    })?;

    let mut segments = vec![];

    while parent_dir.join("package.mo").is_file() {
        let package_name = parent_dir.file_name().ok_or_else(|| {
            anyhow!(
                "Package directory '{}' has no directory name",
                parent_dir.display()
            )
        })?;
        let package_name = package_name.to_str().ok_or_else(|| {
            anyhow!(
                "Package directory '{}' has a non-UTF-8 name",
                parent_dir.display()
            )
        })?;
        segments.push(package_name.to_owned());
        parent_dir = parent_dir
            .parent()
            .ok_or_else(|| {
                anyhow!(
                    "Package directory '{}' has no parent directory",
                    parent_dir.display()
                )
            })?
            .to_path_buf();
    }

    segments.reverse();

    Ok(segments)
}
