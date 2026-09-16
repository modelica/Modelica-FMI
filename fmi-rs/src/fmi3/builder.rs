#![allow(non_snake_case)]

use crate::{
    fmi3::{self, FMU3, log::DefaultLogger},
    sim::SimulationError,
    zip::extract_zip_archive,
};
use std::{
    path::{Path, PathBuf},
    sync::Arc,
};
use tempfile::TempDir;

pub struct FMU3Builder {
    pub unzipdir: TempDir,
    pub model_description: crate::model_description::fmi3::ModelDescription,
    pub visible: bool,
    pub loggingOn: bool,
    pub logFile: Option<PathBuf>,
    pub logCalls: bool,
    pub printMessages: bool,
    pub eventModeUsed: bool,
    pub earlyReturnAllowed: bool,
    pub requiredIntermediateVariables: Vec<u32>,
}

impl FMU3Builder {
    pub fn new<P: AsRef<Path>>(fmu_path: &P) -> Result<Self, SimulationError> {
        let unzipdir = TempDir::new().map_err(SimulationError::io(fmu_path.as_ref()))?;

        extract_zip_archive(fmu_path, &unzipdir)?;

        let model_description = crate::model_description::fmi3::ModelDescription::from_path(
            &unzipdir.path().join("modelDescription.xml"),
        )?;

        Ok(Self {
            unzipdir,
            model_description,
            visible: false,
            loggingOn: false,
            logFile: None,
            logCalls: false,
            printMessages: true,
            eventModeUsed: true,
            earlyReturnAllowed: true,
            requiredIntermediateVariables: vec![],
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

    pub fn printMessages(mut self, printMessages: bool) -> Self {
        self.printMessages = printMessages;
        self
    }

    pub fn instantiate_me(&self, instanceName: &str) -> Result<Arc<FMU3>, SimulationError> {
        if let Some(me) = &self.model_description.modelExchange {
            let logger = Arc::new(if let Some(log_file) = &self.logFile {
                DefaultLogger::from_path(log_file).map_err(SimulationError::io(&log_file))?
            } else {
                DefaultLogger::default()
            });

            fmi3::FMU3::instantiateModelExchange(
                self.unzipdir.path(),
                &me.modelIdentifier,
                instanceName,
                &self.model_description.instantiationToken,
                self.visible,
                self.loggingOn,
                logger,
                self.logCalls,
            )
        } else {
            Err(SimulationError::InterfaceType)
        }
    }

    pub fn instantiate_cs(&self, instanceName: &str) -> Result<Arc<FMU3>, SimulationError> {
        if let Some(cs) = &self.model_description.coSimulation {
            let logger = Arc::new(if let Some(log_file) = &self.logFile {
                DefaultLogger::from_path(log_file).map_err(SimulationError::io(&log_file))?
            } else {
                DefaultLogger::default()
            });

            FMU3::instantiateCoSimulation(
                self.unzipdir.path(),
                &cs.modelIdentifier,
                instanceName,
                &self.model_description.instantiationToken,
                self.visible,
                self.loggingOn,
                self.eventModeUsed,
                self.earlyReturnAllowed,
                logger,
                self.logCalls,
                None,
            )
        } else {
            Err(SimulationError::InterfaceType)
        }
    }
}
