#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]

use serde::{Deserialize, Serialize};
use std::{fs::File, io::BufReader, path::Path};
use thiserror::Error;

/// Fallback type for `<xs:element ref="Annotations" />`.
#[derive(Debug, Serialize, Deserialize, PartialEq, Clone, Default)]
pub struct Annotations;

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
#[serde(rename_all = "camelCase")]
pub struct BuildDescription {
    #[serde(rename = "@fmiVersion")]
    pub fmiVersion: String,

    #[serde(rename = "BuildConfiguration")]
    pub buildConfigurations: Vec<BuildConfiguration>,

    #[serde(rename = "Annotations")]
    pub annotations: Option<Annotations>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
#[serde(rename_all = "camelCase")]
pub struct BuildConfiguration {
    #[serde(rename = "@modelIdentifier")]
    pub modelIdentifier: String,

    #[serde(rename = "@platform")]
    pub platform: Option<String>,

    #[serde(rename = "@description")]
    pub description: Option<String>,

    #[serde(rename = "SourceFileSet", default)]
    pub sourceFileSets: Vec<SourceFileSet>,

    #[serde(rename = "Library", default)]
    pub libraries: Vec<Library>,

    #[serde(rename = "Annotations")]
    pub annotations: Option<Annotations>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
#[serde(rename_all = "camelCase")]
pub struct SourceFileSet {
    #[serde(rename = "@name")]
    pub name: Option<String>,

    #[serde(rename = "@language")]
    pub language: Option<String>,

    #[serde(rename = "@compiler")]
    pub compiler: Option<String>,

    #[serde(rename = "@compilerOptions")]
    pub compilerOptions: Option<String>,

    #[serde(rename = "SourceFile")]
    pub sourceFiles: Vec<SourceFile>,

    #[serde(rename = "PreprocessorDefinition", default)]
    pub preprocessorDefinitions: Vec<PreprocessorDefinition>,

    #[serde(rename = "IncludeDirectory", default)]
    pub includeDirectories: Vec<IncludeDirectory>,

    #[serde(rename = "Annotations")]
    pub annotations: Option<Annotations>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
#[serde(rename_all = "camelCase")]
pub struct SourceFile {
    #[serde(rename = "@name")]
    pub name: String,

    #[serde(rename = "Annotations")]
    pub annotations: Option<Annotations>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
#[serde(rename_all = "camelCase")]
pub struct PreprocessorDefinition {
    #[serde(rename = "@name")]
    pub name: String,

    #[serde(rename = "@optional", default)]
    pub optional: bool,

    #[serde(rename = "@value")]
    pub value: Option<String>,

    #[serde(rename = "@description")]
    pub description: Option<String>,

    #[serde(rename = "Option", default)]
    pub options: Vec<PreprocessorOption>,

    #[serde(rename = "Annotations")]
    pub annotations: Option<Annotations>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
#[serde(rename_all = "camelCase")]
pub struct PreprocessorOption {
    #[serde(rename = "@value")]
    pub value: Option<String>,

    #[serde(rename = "@description")]
    pub description: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
#[serde(rename_all = "camelCase")]
pub struct IncludeDirectory {
    #[serde(rename = "@name")]
    pub name: String,

    #[serde(rename = "Annotations")]
    pub annotations: Option<Annotations>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
#[serde(rename_all = "camelCase")]
pub struct Library {
    #[serde(rename = "@name")]
    pub name: String,

    #[serde(rename = "@version")]
    pub version: Option<String>,

    #[serde(rename = "@external", default = "bool::default")]
    pub external: bool,

    #[serde(rename = "@description")]
    pub description: Option<String>,

    #[serde(rename = "Annotations")]
    pub annotations: Option<Annotations>,
}

#[derive(Error, Debug)]
pub enum BuildDescriptionError {
    #[error("Failed to open the file")]
    Io(#[from] std::io::Error),

    #[error("Failed to parse the file: {0}")]
    Parse(String),
}

impl BuildDescription {
    pub fn from_file<P: AsRef<Path>>(path: P) -> Result<Self, BuildDescriptionError> {
        let file = File::open(path)?;
        let reader = BufReader::new(file);
        let build_description: BuildDescription = quick_xml::de::from_reader(reader)
            .map_err(|e| BuildDescriptionError::Parse(e.to_string()))?;
        Ok(build_description)
    }
}
