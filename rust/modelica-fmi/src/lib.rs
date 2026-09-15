use anyhow::anyhow;
use std::{fs, path::Path};

pub fn modelica_within_path(model_path: &Path) -> anyhow::Result<String> {
    let model_parent = model_path.parent().ok_or(anyhow!("d'oh!"))?;

    let mut parent_dir = fs::canonicalize(model_parent)?;

    let mut segments = vec![];

    while parent_dir.join("package.mo").is_file() {
        let package_name = parent_dir.file_name().ok_or(anyhow!("d'oh!"))?;
        segments.push(package_name.to_str().ok_or(anyhow!("d'oh!"))?.to_owned());
        parent_dir = parent_dir.parent().ok_or(anyhow!("d'oh!"))?.to_path_buf();
    }

    segments.reverse();

    Ok(segments.join("."))
}
