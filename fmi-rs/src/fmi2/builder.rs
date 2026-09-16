#![allow(non_snake_case)]

use crate::{fmi2, sim::SimulationError, zip::extract_zip_archive};
use std::{
    path::{Path, PathBuf},
    sync::Arc,
};
use tempfile::TempDir;

pub struct FMU2Builder {
    pub unzipdir: TempDir,
    pub model_description: crate::model_description::fmi2::ModelDescription,
    pub visible: bool,
    pub loggingOn: bool,
    pub logFile: Option<PathBuf>,
    pub logCalls: bool,
}

impl FMU2Builder {
    pub fn new<P: AsRef<Path>>(fmu_path: &P) -> Result<Self, SimulationError> {
        let unzipdir = TempDir::new().map_err(SimulationError::io(fmu_path.as_ref()))?;

        extract_zip_archive(fmu_path, &unzipdir)?;

        let model_description = crate::model_description::fmi2::ModelDescription::from_path(
            &unzipdir.path().join("modelDescription.xml"),
        )?;

        Ok(Self {
            unzipdir,
            model_description,
            visible: false,
            loggingOn: false,
            logFile: None,
            logCalls: false,
        })
    }

    pub fn visible(mut self, visible: bool) -> Self {
        self.visible = visible;
        self
    }

    pub fn loggingOn(mut self, loggingOn: bool) -> Self {
        self.loggingOn = loggingOn;
        self
    }

    pub fn logCalls(mut self, logCalls: bool) -> Self {
        self.logCalls = logCalls;
        self
    }

    pub fn instantiate_me(
        &self,
        instanceName: &str,
    ) -> Result<Arc<fmi2::FMU2<fmi2::ME>>, SimulationError> {
        if let Some(me) = &self.model_description.modelExchange {
            let logger = Arc::new(if let Some(log_file) = &self.logFile {
                fmi2::log::DefaultLogger::from_path(log_file)
                    .map_err(SimulationError::io(&log_file))?
            } else {
                fmi2::log::DefaultLogger::default()
            });

            fmi2::FMU2::<fmi2::ME>::new(
                self.unzipdir.path(),
                &me.modelIdentifier,
                instanceName,
                &self.model_description.guid,
                self.visible,
                self.loggingOn,
                self.logCalls,
                logger,
                !me.canNotUseMemoryManagementFunctions,
            )
        } else {
            Err(SimulationError::InterfaceType)
        }
    }

    pub fn instantiate_cs(
        &self,
        instanceName: &str,
    ) -> Result<Arc<fmi2::FMU2<fmi2::CS>>, SimulationError> {
        if let Some(cs) = &self.model_description.coSimulation {
            let logger = Arc::new(if let Some(log_file) = &self.logFile {
                fmi2::log::DefaultLogger::from_path(log_file)
                    .map_err(SimulationError::io(&log_file))?
            } else {
                fmi2::log::DefaultLogger::default()
            });

            fmi2::FMU2::<fmi2::CS>::new(
                self.unzipdir.path(),
                &cs.modelIdentifier,
                instanceName,
                &self.model_description.guid,
                self.visible,
                self.loggingOn,
                self.logCalls,
                logger,
                !cs.canNotUseMemoryManagementFunctions,
            )
        } else {
            Err(SimulationError::InterfaceType)
        }
    }
}
