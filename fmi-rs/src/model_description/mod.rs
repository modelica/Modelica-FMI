#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]

mod file;
pub mod fmi2;
pub mod fmi3;
pub mod validation;

use std::{ops::Range, path::Path};
use thiserror::Error;

/// Represents a problem found during model description validation.
#[derive(Debug)]
pub struct ValidationError {
    pub range: Vec<Range<usize>>,
    pub message: String,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone, Copy)]
pub enum FMIMajorVersion {
    V2 = 2,
    V3 = 3,
}

#[derive(Error, Debug)]
pub enum ModelDescriptionError {
    #[error("Failed to open the file")]
    Io(#[from] std::io::Error),

    #[error("{0}")]
    Parse(String),

    #[error("Missing attribute '{0}'")]
    MissingAttribute(String),

    #[error("Unsupported FMI version '{0}'")]
    UnsupportedVersion(String),

    #[error("Unknown FMI version '{0}'")]
    UnknownVersion(String),

    #[error("Illegal value reference: {0}")]
    ValueReference(u32),

    #[error("Illegal variable index: {0}")]
    VariableIndex(u32),

    #[error("Illegal variable name: {0}")]
    VariableName(String),
}

#[derive(Debug)]
pub struct Unit {
    pub name: String,
    pub baseUnit: Option<BaseUnit>,
    pub displayUnits: Vec<DisplayUnit>,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct DisplayUnit {
    pub name: String,
    pub factor: f64,
    pub offset: f64,
    pub inverse: bool,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct BaseUnit {
    pub kg: i32,
    pub m: i32,
    pub s: i32,
    pub A: i32,
    pub K: i32,
    pub mol: i32,
    pub cd: i32,
    pub rad: i32,
    pub factor: f64,
    pub offset: f64,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct Category {
    pub name: String,
    pub description: Option<String>,
}

#[derive(Debug)]
pub struct DefaultExperiment {
    pub startTime: Option<String>,
    pub stopTime: Option<String>,
    pub tolerance: Option<String>,
    pub stepSize: Option<String>,
    pub range: Range<usize>,
}

pub fn peek_fmi_version(path: &Path) -> Result<String, ModelDescriptionError> {
    let text = std::fs::read_to_string(path)?;

    let opt = roxmltree::ParsingOptions {
        allow_dtd: true,
        ..roxmltree::ParsingOptions::default()
    };

    let doc = roxmltree::Document::parse_with_options(&text, opt)
        .map_err(|err| ModelDescriptionError::Parse(err.to_string()))?;

    let root = doc.root_element();

    if let Some(fmi_version) = root.attribute("fmiVersion") {
        Ok(fmi_version.to_string())
    } else {
        Err(ModelDescriptionError::MissingAttribute(
            "fmiVersion".to_string(),
        ))
    }
}

pub fn peek_fmi_major_version(path: &Path) -> Result<FMIMajorVersion, ModelDescriptionError> {
    let fmi_version = peek_fmi_version(path)?;

    if fmi_version == "1.0" {
        Err(ModelDescriptionError::UnsupportedVersion(fmi_version))
    } else if fmi_version == "2.0" {
        Ok(FMIMajorVersion::V2)
    } else if fmi_version.starts_with("3.") {
        Ok(FMIMajorVersion::V3)
    } else {
        Err(ModelDescriptionError::UnknownVersion(fmi_version))
    }
}
