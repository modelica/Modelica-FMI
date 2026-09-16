#![allow(
    non_camel_case_types,
    non_snake_case,
    non_upper_case_globals,
    clippy::too_many_arguments
)]

pub mod fmi2;
pub mod fmi3;
pub mod solver;

use std::{ffi::NulError, path::PathBuf, slice::SliceIndex};

use approx::{relative_eq, relative_ne};
use thiserror::Error;

use crate::dae::DaeManifestError;
#[cfg(feature = "zip")]
use crate::{model_description::ModelDescriptionError, zip::ZipError};

#[derive(Debug, Error)]
pub enum SimulationError {
    #[error("Failed to load model description: {0}")]
    ModelDescription(#[from] ModelDescriptionError),

    #[error("Failed to load FMI-LS-DAE manifest: {0}")]
    DaeManifest(#[from] DaeManifestError),

    #[error("Failed to load platform binary '{path}': {source}")]
    Library {
        path: PathBuf,
        source: libloading::Error,
    },

    #[error("Failed to load symbol '{name}' from platform binary: {source}")]
    Symbol {
        name: String,
        source: libloading::Error,
    },

    #[error("Failed to open '{path}': {source}")]
    Io {
        path: PathBuf,
        source: std::io::Error,
    },

    #[error("FMI call failed")]
    FMICall,

    #[error("Interface type not supported")]
    InterfaceType,

    #[error("Illegal simulation parameter: {0}")]
    Parameter(String),

    #[error("String contains a NUL byte: {0}")]
    Nul(#[from] NulError),

    #[error(
        "The next event time ({next_event_time}) must be greater than the current time ({time})"
    )]
    NextEventTime { time: f64, next_event_time: f64 },

    #[error("Failed to parse value: {0}")]
    Parse(String),

    #[error("Solver error: {0}")]
    Solver(String),

    #[cfg(feature = "zip")]
    #[error("Failed to extract Zip archive: {0}")]
    Zip(#[from] ZipError),

    #[error("Index out of bounds")]
    IndexOutOfBounds,
}

impl SimulationError {
    /// Helper to easily wrap an IO error with its path
    pub fn io(path: impl Into<std::path::PathBuf>) -> impl FnOnce(std::io::Error) -> Self {
        let path = path.into();
        move |source| SimulationError::Io { source, path }
    }
}

pub trait SimulationSliceExt<T> {
    // fn try_set(&mut self, index: I, value: T) -> Result<(), SimulationError>;
    fn try_get<I>(&self, index: I) -> Result<&I::Output, SimulationError>
    where
        I: SliceIndex<[T]>;
    fn try_get_mut<I>(&mut self, index: I) -> Result<&mut I::Output, SimulationError>
    where
        I: SliceIndex<[T]>;
    fn set(&mut self, index: usize, value: T) -> Result<(), SimulationError>;
    // fn try_get_mut(&mut self, index: usize) -> Result<&mut T, SimulationError>;
    // fn try_get_slice(&self, range: Range<usize>) -> Result<&[T], SimulationError>;
    // fn try_get_slice_mut(&mut self, range: Range<usize>) -> Result<&mut [T], SimulationError>;
}

impl<T> SimulationSliceExt<T> for [T] {
    fn try_get<I>(&self, index: I) -> Result<&I::Output, SimulationError>
    where
        I: SliceIndex<[T]>,
    {
        self.get(index).ok_or(SimulationError::IndexOutOfBounds)
    }

    fn try_get_mut<I>(&mut self, index: I) -> Result<&mut I::Output, SimulationError>
    where
        I: SliceIndex<[T]>,
    {
        self.get_mut(index).ok_or(SimulationError::IndexOutOfBounds)
    }

    fn set(&mut self, index: usize, value: T) -> Result<(), SimulationError> {
        let slot = self
            .get_mut(index)
            .ok_or(SimulationError::IndexOutOfBounds)?;
        *slot = value;
        Ok(())
    }

    // fn try_get_mut(&mut self, index: usize) -> Result<&mut T, SimulationError> {
    //     self.get_mut(index)
    //         .ok_or(SimulationError::IndexOutOfBounds)
    // }

    // fn try_get_slice(&self, range: std::ops::Range<usize>) -> Result<&[T], SimulationError> {
    //     self.get(range)
    //         .ok_or(SimulationError::IndexOutOfBounds)
    // }

    // fn try_get_slice_mut(&mut self, range: std::ops::Range<usize>) -> Result<&mut [T], SimulationError> {
    //     self.get_mut(range)
    //         .ok_or(SimulationError::IndexOutOfBounds)
    // }
}

pub type SetTimeFn<'a> = Box<dyn Fn(f64) -> Result<(), SimulationError> + 'a>;
pub type SetContinuousInputsFn<'a> = Box<dyn Fn(f64) -> Result<(), SimulationError> + 'a>;
pub type GetEventIndicatorsFn<'a> = Box<dyn Fn(&mut [f64]) -> Result<(), SimulationError> + 'a>;
pub type GetContinuousStatesFn<'a> = Box<dyn Fn(&mut [f64]) -> Result<(), SimulationError> + 'a>;
pub type GetNominalsOfContinuousStatesFn<'a> =
    Box<dyn Fn(&mut [f64]) -> Result<(), SimulationError> + 'a>;
pub type GetContinuousStateDerivativesFn<'a> =
    Box<dyn Fn(&mut [f64]) -> Result<(), SimulationError> + 'a>;
pub type GetDirectionalDerivativeFn<'a> =
    Box<dyn Fn(&[u32], &[u32], &[f64], &mut [f64]) -> Result<(), SimulationError> + 'a>;
pub type SetContinuousStatesFn<'a> = Box<dyn Fn(&[f64]) -> Result<(), SimulationError> + 'a>;

/// Approximate equality using both the absolute difference and relative based comparisons.
pub fn relative_eq(lhs: f64, rhs: f64, relative_tolerance: f64) -> bool {
    relative_eq!(lhs, rhs, max_relative = relative_tolerance)
}

/// Approximate inequality using both the absolute difference and relative based comparisons.
pub fn relative_ne(lhs: f64, rhs: f64, relative_tolerance: f64) -> bool {
    relative_ne!(lhs, rhs, max_relative = relative_tolerance)
}

/// Greater or approximate equality using both the absolute difference and relative based comparisons.
pub fn relative_ge(lhs: f64, rhs: f64, relative_tolerance: f64) -> bool {
    lhs > rhs || relative_eq(lhs, rhs, relative_tolerance)
}

/// Less or approximate equality using both the absolute difference and relative based comparisons.
pub fn relative_le(lhs: f64, rhs: f64, relative_tolerance: f64) -> bool {
    lhs < rhs || relative_eq(lhs, rhs, relative_tolerance)
}

/// Less than and not approximate equality using both the absolute difference and relative based comparisons.
pub fn relative_lt(lhs: f64, rhs: f64, relative_tolerance: f64) -> bool {
    lhs < rhs && !relative_eq(lhs, rhs, relative_tolerance)
}

/// Greater than and not approximate equality using both the absolute difference and relative based comparisons.
pub fn relative_gt(lhs: f64, rhs: f64, relative_tolerance: f64) -> bool {
    lhs > rhs && !relative_eq(lhs, rhs, relative_tolerance)
}

/// Validates the simulation steps and returns an error message if any of the checks fail.
pub fn validate_simulation_steps(
    start_time: f64,
    stop_time: f64,
    output_interval: f64,
    relative_tolerance: f64,
) -> Result<(), String> {
    if stop_time < start_time {
        return Err(format!(
            "Stop time ({}) must be greater than or equal to start time ({}).",
            stop_time, start_time
        ));
    }

    if output_interval <= 0.0 {
        return Err(format!(
            "Output interval ({}) must be greater than 0.",
            output_interval
        ));
    } else if output_interval > (stop_time - start_time) {
        return Err(format!(
            "Output interval ({}) must be less than or equal to the simulation duration ({}).",
            output_interval,
            stop_time - start_time
        ));
    } else if !relative_eq(
        ((stop_time - start_time) / output_interval).fract(),
        0.0,
        relative_tolerance,
    ) {
        return Err(format!(
            "Output interval ({}) must be a divisor of the simulation duration ({}).",
            output_interval,
            stop_time - start_time
        ));
    }

    Ok(())
}

/// Calculates the next communication point
pub fn next_communication_point(
    next_regular_point: f64,
    next_input_event_time: Option<f64>,
    next_event_time: Option<f64>,
    stop_time: f64,
    relative_tolerance: f64,
) -> f64 {
    let mut next_communication_point = next_regular_point;

    if let Some(next_input_event_time) = next_input_event_time
        && relative_gt(
            next_regular_point,
            next_input_event_time,
            relative_tolerance,
        )
    {
        next_communication_point = next_input_event_time;
    }

    if let Some(next_event_time) = next_event_time
        && relative_gt(
            next_communication_point,
            next_event_time,
            relative_tolerance,
        )
    {
        next_communication_point = next_event_time;
    }

    if relative_gt(next_communication_point, stop_time, relative_tolerance) {
        next_communication_point = stop_time;
    }

    next_communication_point
}

/// Calculates the next regular sample point
pub fn next_regular_point(
    log_time_scale: bool,
    start_time: f64,
    output_interval: f64,
    n_steps: i32,
) -> Result<f64, SimulationError> {
    if log_time_scale {
        if output_interval <= 1.0 {
            Err(SimulationError::Parameter(format!(
                "Expected output_interval > 1 for logarithmic time scale but got {output_interval}"
            )))
        } else {
            Ok(start_time * output_interval.powi(n_steps.saturating_add(1)))
        }
    } else if output_interval <= 0.0 {
        Err(SimulationError::Parameter(format!(
            "Expected output_interval > 0 but got {output_interval}"
        )))
    } else {
        Ok(start_time + n_steps.saturating_add(1) as f64 * output_interval)
    }
}
