use crate::{
    model_description::{self, peek_fmi_major_version},
    zip::{ZipError, extract_zip_archive},
};
use askama::Template;
use std::{
    fs::{self},
    path::{self, Path, PathBuf},
};

#[derive(Template)]
#[template(path = "CMakeLists.txt")]
struct CMakeListsTemplate {
    model_identifier: String,
    fmi_major_version: i32,
    definitions: Vec<String>,
    sources: Vec<String>,
    include_dirs: Vec<String>,
    target_path: String,
}

const FMI2_FUNCTIONS_H: &[u8] = include_bytes!("../templates/fmi2Functions.h");
const FMI2_FUNCTION_TYPES_H: &[u8] = include_bytes!("../templates/fmi2FunctionTypes.h");
const FMI2_TYPES_PLATFORM_H: &[u8] = include_bytes!("../templates/fmi2TypesPlatform.h");

const FMI3_FUNCTIONS_H: &[u8] = include_bytes!("../templates/fmi3Functions.h");
const FMI3_FUNCTION_TYPES_H: &[u8] = include_bytes!("../templates/fmi3FunctionTypes.h");
const FMI3_PLATFORM_TYPES_H: &[u8] = include_bytes!("../templates/fmi3PlatformTypes.h");

use thiserror::Error;

#[derive(Error, Debug)]
pub enum CMakeProjectError {
    #[error("Failed to read the input FMU archive")]
    Zip(#[from] ZipError),

    #[error("Model description error: {0}")]
    ModelDescription(#[from] crate::model_description::ModelDescriptionError),

    #[error("IO error occurred while creating project at {path}")]
    Io {
        path: PathBuf,
        #[source]
        source: std::io::Error,
    },

    #[error("{0}")]
    Message(String),

    #[error("Template rendering error: {0}")]
    Template(#[from] askama::Error),
    // #[error("CMake generation failed: {reason}")]
    // GenerationFailed { reason: String },
}

impl From<std::io::Error> for CMakeProjectError {
    fn from(source: std::io::Error) -> Self {
        Self::Io {
            path: PathBuf::new(),
            source,
        }
    }
}

impl From<String> for CMakeProjectError {
    fn from(message: String) -> Self {
        Self::Message(message)
    }
}

impl From<&str> for CMakeProjectError {
    fn from(message: &str) -> Self {
        Self::Message(message.to_string())
    }
}

impl CMakeProjectError {
    /// Helper to cleanly map IO errors associated with a specific path
    pub fn io_at(path: &std::path::Path) -> impl FnOnce(std::io::Error) -> Self + '_ {
        move |source| CMakeProjectError::Io {
            path: path.to_path_buf(),
            source,
        }
    }
}

pub fn create_cmake_project(fmu_path: &Path, project_path: &Path) -> Result<(), CMakeProjectError> {
    let source_path = project_path.join("src");
    std::fs::create_dir_all(&source_path).map_err(CMakeProjectError::io_at(&source_path))?;

    extract_zip_archive(fmu_path, &source_path)?;

    let include_path = project_path.join("include");
    std::fs::create_dir_all(&include_path).map_err(CMakeProjectError::io_at(&source_path))?;

    let model_description_path = source_path.join("modelDescription.xml");

    let fmi_major_version = peek_fmi_major_version(&model_description_path)?;

    let target_path = path::absolute(fmu_path)
        .map_err(CMakeProjectError::io_at(fmu_path))?
        .to_string_lossy()
        .replace("\\", "/");

    let cmake_lists_template = match fmi_major_version {
        model_description::FMIMajorVersion::V2 => {
            let model_description = crate::model_description::fmi2::ModelDescription::from_path(
                model_description_path.as_path(),
            )
            .map_err(|e| format!("Failed to parse model description: {}", e))?;

            fs::write(include_path.join("fmi2Functions.h"), FMI2_FUNCTIONS_H)?;
            fs::write(
                include_path.join("fmi2FunctionTypes.h"),
                FMI2_FUNCTION_TYPES_H,
            )?;
            fs::write(
                include_path.join("fmi2TypesPlatform.h"),
                FMI2_TYPES_PLATFORM_H,
            )?;

            let mut sources = vec![
                "src/modelDescription.xml".to_string(),
                "include/fmi2Functions.h".to_string(),
                "include/fmi2FunctionTypes.h".to_string(),
                "include/fmi2TypesPlatform.h".to_string(),
            ];

            let model_identifier = if let Some(cs) = &model_description.coSimulation {
                for file in &cs.sourceFiles {
                    sources.push(format!("src/sources/{}", file));
                }
                cs.modelIdentifier.clone()
            } else if let Some(me) = &model_description.modelExchange {
                for file in &me.sourceFiles {
                    sources.push(format!("src/sources/{}", file));
                }
                me.modelIdentifier.clone()
            } else {
                return Err("No model identifier found in modelDescription.xml".into());
            };

            let include_dirs = vec!["include".to_string(), "src/sources".to_string()];

            CMakeListsTemplate {
                model_identifier,
                fmi_major_version: 2,
                definitions: vec![],
                sources,
                include_dirs,
                target_path,
            }
        }
        model_description::FMIMajorVersion::V3 => {
            let model_description = crate::model_description::fmi3::ModelDescription::from_path(
                model_description_path.as_path(),
            )
            .map_err(|e| format!("Failed to parse model description: {}", e))?;

            let build_description_path = source_path.join("sources/buildDescription.xml");

            let build_description =
                crate::build_description::BuildDescription::from_file(build_description_path)
                    .map_err(|e| format!("Failed to parse build description: {}", e))?;

            fs::write(include_path.join("fmi3Functions.h"), FMI3_FUNCTIONS_H)?;
            fs::write(
                include_path.join("fmi3FunctionTypes.h"),
                FMI3_FUNCTION_TYPES_H,
            )?;
            fs::write(
                include_path.join("fmi3PlatformTypes.h"),
                FMI3_PLATFORM_TYPES_H,
            )?;

            let mut definitions = vec![
                "FMI3_OVERRIDE_FUNCTION_PREFIX".to_string(),
                "FMI3_ACTUAL_FUNCTION_PREFIX=\"\"".to_string(),
            ];

            #[cfg(target_os = "windows")]
            definitions.push("FMI3_Export=__declspec(dllexport)".to_string());

            let mut include_dirs = vec!["include".to_string()];

            let mut sources = vec!["src/modelDescription.xml".to_string()];

            for build_configuration in build_description.buildConfigurations {
                for source_file_set in build_configuration.sourceFileSets {
                    for definition in &source_file_set.preprocessorDefinitions {
                        if let Some(value) = &definition.value {
                            definitions.push(format!("{}={}", definition.name, value));
                        } else {
                            definitions.push(definition.name.clone());
                        }
                    }
                    for include_directory in &source_file_set.includeDirectories {
                        include_dirs.push(format!("src/sources/{}", include_directory.name));
                    }
                    for source_file in &source_file_set.sourceFiles {
                        sources.push(format!("src/sources/{}", source_file.name));
                    }
                }
            }

            let model_identifier = if let Some(cs) = &model_description.coSimulation {
                cs.modelIdentifier.clone()
            } else if let Some(me) = &model_description.modelExchange {
                me.modelIdentifier.clone()
            } else {
                return Err("No model identifier found in modelDescription.xml".into());
            };

            CMakeListsTemplate {
                model_identifier,
                fmi_major_version: 3,
                definitions,
                sources,
                include_dirs,
                target_path,
            }
        }
    };

    let cmake_lists_path = project_path.join("CMakeLists.txt");

    fs::write(cmake_lists_path, cmake_lists_template.render()?)?;

    Ok(())
}
