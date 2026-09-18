use anyhow::anyhow;
use sha2::{Digest, Sha256};
use std::{fs, io, path::Path};
use std::{fs::File, io::Read};

pub fn modelica_path(model_path: &Path) -> anyhow::Result<Vec<String>> {
    let model_parent = model_path.parent().ok_or_else(|| {
        anyhow!(
            "Model path '{}' has no parent directory",
            model_path.display()
        )
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

pub fn update_package_order(output_file: &Path) -> io::Result<()> {
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

pub fn sha256_file(path: &Path) -> io::Result<String> {
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

pub fn modelica_identifier(value: &str) -> String {
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

pub fn is_modelica_identifier(value: &str) -> bool {
    let mut characters = value.chars();

    matches!(
        characters.next(),
        Some(character) if character == '_' || character.is_ascii_alphabetic()
    ) && characters.all(|character| character == '_' || character.is_ascii_alphanumeric())
}

pub fn port_annotation(icon_width: f64, icon_height: f64, n_ports: usize, i: usize, is_input: bool) -> String {

    let y_off = icon_height / (n_ports as f64 + 1.0);

    let y = (icon_height / 2.) - (i as f64 + 0.5) * y_off;

    let x1 = if is_input { -(icon_width / 2.) - 20. } else { icon_width / 2. };
    let x2 = x1 + 20.;
    let y1 = y - 10. - y_off / 2.0;
    let y2 = y + 10. - y_off / 2.0;

    format!(
        " annotation(Placement(transformation(extent={{ {{ {x1}, {y1} }}, {{ {x2}, {y2} }} }}), iconTransformation(extent={{ {{ {x1}, {y1} }}, {{ {x2}, {y2} }} }})))"
    )
}

pub fn port_label(icon_width: f64, icon_height: f64, n_ports: usize, i: usize, is_input: bool, text: &str) -> String {

    let y_off = icon_height / (n_ports as f64 + 1.0);

    let y = (icon_height / 2.) - (i as f64 + 0.5) * y_off;

    let x1 = if is_input { -(icon_width / 2.) + 10. } else { icon_width / 2. - 10. };
    let y1 = y - 10. - y_off / 2.0;
    let y2 = y + 10. - y_off / 2.0;

    let alignment = if is_input { "Left" } else { "Right" };

    format!(
        "Text(extent={{ {{ {x1}, {y1} }}, {{ {x1}, {y2} }} }}, textColor={{0,0,0}}, textString=\"{text}\", horizontalAlignment=TextAlignment.{alignment}, visible=showLabels)",
    )
}
