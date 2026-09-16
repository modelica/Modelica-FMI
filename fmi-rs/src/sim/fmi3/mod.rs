pub mod cs;
pub mod csv;
pub mod dae;
pub mod input;
pub mod me;
pub mod recorder;

use std::sync::Arc;
use std::{fs, ptr};

use crate::model_description::fmi3::{Causality, ModelDescription};
use crate::sim::SimulationError;
use crate::{
    fmi3::{FMU3, types::*},
    model_description::fmi3::{ModelVariable, VariableType},
};

use std::path::{Path, PathBuf};

pub struct SimulationSettings {
    pub unzipdir: PathBuf,
    pub model_description: Arc<ModelDescription>,
    pub enable_dae: bool,
    pub start_time: f64,
    pub stop_time: f64,
    pub logging_on: bool,
    pub set_stop_time: bool,
    pub output_interval: f64,
    pub log_time_scale: bool,
    pub tolerance: f64,
    pub set_tolerance: bool,
    pub start_values: Vec<(String, String)>,
    pub log_fmi_calls: bool,
    pub intermediate_update: bool,
    pub input_file: Option<PathBuf>,
    pub early_return_allowed: bool,
    pub event_mode_used: bool,
    pub log_file: Option<PathBuf>,
    pub initial_fmu_state_file: Option<PathBuf>,
    pub final_fmu_state_file: Option<PathBuf>,
}

#[derive(Debug, PartialEq)]
pub enum VariableValue {
    Float32(Vec<fmi3Float32>),
    Float64(Vec<fmi3Float64>),
    Int8(Vec<fmi3Int8>),
    UInt8(Vec<fmi3UInt8>),
    Int16(Vec<fmi3Int16>),
    UInt16(Vec<fmi3UInt16>),
    Int32(Vec<fmi3Int32>),
    UInt32(Vec<fmi3UInt32>),
    Int64(Vec<fmi3Int64>),
    UInt64(Vec<fmi3UInt64>),
    Boolean(Vec<fmi3Boolean>),
    String(Vec<String>),
    Binary(Vec<Vec<fmi3Byte>>),
    // Clock(fmiClock),
}

impl VariableValue {
    pub fn len(&self) -> usize {
        match self {
            VariableValue::Float32(v) => v.len(),
            VariableValue::Float64(v) => v.len(),
            VariableValue::Int8(v) => v.len(),
            VariableValue::UInt8(v) => v.len(),
            VariableValue::Int16(v) => v.len(),
            VariableValue::UInt16(v) => v.len(),
            VariableValue::Int32(v) => v.len(),
            VariableValue::UInt32(v) => v.len(),
            VariableValue::Int64(v) => v.len(),
            VariableValue::UInt64(v) => v.len(),
            VariableValue::Boolean(v) => v.len(),
            VariableValue::String(v) => v.len(),
            VariableValue::Binary(v) => v.len(),
        }
    }

    pub fn is_empty(&self) -> bool {
        match self {
            VariableValue::Float32(v) => v.is_empty(),
            VariableValue::Float64(v) => v.is_empty(),
            VariableValue::Int8(v) => v.is_empty(),
            VariableValue::UInt8(v) => v.is_empty(),
            VariableValue::Int16(v) => v.is_empty(),
            VariableValue::UInt16(v) => v.is_empty(),
            VariableValue::Int32(v) => v.is_empty(),
            VariableValue::UInt32(v) => v.is_empty(),
            VariableValue::Int64(v) => v.is_empty(),
            VariableValue::UInt64(v) => v.is_empty(),
            VariableValue::Boolean(v) => v.is_empty(),
            VariableValue::String(v) => v.is_empty(),
            VariableValue::Binary(v) => v.is_empty(),
        }
    }

    pub fn to_literal(&self) -> String {
        match self {
            VariableValue::Float32(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::Float64(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::Int8(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::UInt8(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::Int16(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::UInt16(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::Int32(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::UInt32(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::Int64(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::UInt64(v) => v
                .iter()
                .map(|x| x.to_string())
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::Boolean(v) => v
                .iter()
                .map(|&b| if b { "true" } else { "false" })
                .collect::<Vec<_>>()
                .join(" "),
            VariableValue::String(v) => v.join(" "),
            VariableValue::Binary(v) => v
                .iter()
                .map(|bytes| {
                    bytes
                        .iter()
                        .map(|b| format!("{:02x}", b))
                        .collect::<String>()
                })
                .collect::<Vec<_>>()
                .join(" "),
        }
    }

    pub fn as_f64(&self) -> Option<Vec<f64>> {
        match self {
            VariableValue::Float32(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::Float64(v) => Some(v.clone()),
            VariableValue::Int8(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::UInt8(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::Int16(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::UInt16(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::Int32(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::UInt32(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::Int64(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::UInt64(v) => Some(v.iter().map(|x| *x as f64).collect()),
            VariableValue::Boolean(v) => {
                Some(v.iter().map(|&b| if b { 1.0 } else { 0.0 }).collect())
            }
            VariableValue::String(_) | VariableValue::Binary(_) => None,
        }
    }
}

#[derive(Debug)]
pub struct Trajectories {
    pub model_description: Arc<ModelDescription>,
    pub variable_indices: Vec<usize>,
    pub time: Vec<f64>,
    pub rows: Vec<Vec<VariableValue>>,
}

impl Trajectories {
    pub fn new(model_description: Arc<ModelDescription>, variable_indices: Vec<usize>) -> Self {
        Trajectories {
            model_description,
            variable_indices,
            time: vec![],
            rows: vec![],
        }
    }

    pub fn value_references(&self) -> impl Iterator<Item = fmi3ValueReference> {
        self.variable_indices.iter().copied().filter_map(|idx| {
            self.model_description
                .modelVariables
                .get(idx)
                .map(|v| v.valueReference)
        })
    }

    pub fn variables(&self) -> impl Iterator<Item = &ModelVariable> {
        self.variable_indices
            .iter()
            .copied()
            .filter_map(|idx| self.model_description.modelVariables.get(idx))
    }

    pub fn len(&self) -> usize {
        self.variable_indices.len()
    }

    pub fn is_empty(&self) -> bool {
        self.variable_indices.is_empty()
    }

    pub fn step_count(&self) -> usize {
        self.time.len()
    }

    /// Validates the structural integrity and data consistency of the trajectories.
    pub fn validate(&self) -> Result<(), String> {
        if self.time.len() != self.rows.len() {
            return Err(format!(
                "Time vector length ({}) does not match rows length ({}).",
                self.time.len(),
                self.rows.len()
            ));
        }

        for (i, window) in self.time.windows(2).enumerate() {
            if let &[t0, t1] = window
                && t0 > t1
            {
                return Err(format!(
                    "Time is decreasing at row {} ({t0} -> {t1}).",
                    i.saturating_add(2)
                ));
            }
        }

        for (i, row) in self.rows.iter().enumerate() {
            if row.len() != self.variable_indices.len() {
                return Err(format!(
                    "Row {i} has {} columns, but {} variables are defined.",
                    row.len(),
                    self.variable_indices.len()
                ));
            }
        }

        Ok(())
    }

    /// Return a list of all event times
    pub fn events(&self) -> Vec<f64> {
        let mut events = vec![];

        for w in self.time.windows(2) {
            if let &[t0, t1] = w
                && t0 == t1
                && events.last().copied() != Some(t0)
            {
                events.push(t0);
            }
        }

        events
    }
}

pub fn parse_variable_value(
    variable_type: &VariableType,
    literal: &str,
) -> Result<VariableValue, SimulationError> {
    match variable_type {
        VariableType::Float32 { .. } => {
            let values: Result<Vec<fmi3Float32>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::Float32(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::Float64 { .. } => {
            let values: Result<Vec<fmi3Float64>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::Float64(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::Int8 { .. } => {
            let values: Result<Vec<fmi3Int8>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::Int8(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::UInt8 { .. } => {
            let values: Result<Vec<fmi3UInt8>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::UInt8(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::Int16 { .. } => {
            let values: Result<Vec<fmi3Int16>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::Int16(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::UInt16 { .. } => {
            let values: Result<Vec<fmi3UInt16>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::UInt16(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::Int32 { .. } => {
            let values: Result<Vec<fmi3Int32>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::Int32(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::UInt32 { .. } => {
            let values: Result<Vec<fmi3UInt32>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::UInt32(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::Int64 { .. } | VariableType::Enumeration { .. } => {
            let values: Result<Vec<fmi3Int64>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::Int64(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::UInt64 { .. } => {
            let values: Result<Vec<fmi3UInt64>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::UInt64(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::Boolean { .. } | VariableType::Clock { .. } => {
            let values: Result<Vec<fmi3Boolean>, _> =
                literal.split_whitespace().map(|v| v.parse()).collect();
            Ok(VariableValue::Boolean(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
        VariableType::String { .. } => {
            let values: Vec<String> = literal.split_whitespace().map(|v| v.to_string()).collect();
            Ok(VariableValue::String(values))
        }
        VariableType::Binary { .. } => {
            let values: Result<Vec<Vec<fmi3Byte>>, SimulationError> = literal
                .split_whitespace()
                .map(|hex_str| {
                    if hex_str.len() % 2 != 0 {
                        return Err(SimulationError::Parse(format!(
                            "Invalid hex string length: {}",
                            hex_str
                        )));
                    }

                    let mut bytes = Vec::new();

                    for pair in hex_str.as_bytes().as_chunks::<2>().0 {
                        let byte_str = std::str::from_utf8(pair).map_err(|error| {
                            SimulationError::Parse(format!("Invalid hex byte: {error}"))
                        })?;
                        match u8::from_str_radix(byte_str, 16) {
                            Ok(byte) => bytes.push(byte),
                            Err(e) => {
                                return Err(SimulationError::Parse(format!(
                                    "Invalid hex byte '{}': {}",
                                    byte_str, e
                                )));
                            }
                        }
                    }

                    Ok(bytes)
                })
                .collect();
            Ok(VariableValue::Binary(
                values.map_err(|e| SimulationError::Parse(e.to_string()))?,
            ))
        }
    }
}

pub fn set_variable_value(
    fmu: &FMU3,
    value_reference: fmi3ValueReference,
    value: &VariableValue,
) -> fmi3Status {
    match value {
        VariableValue::Float32(values) => fmu.setFloat32(&[value_reference], values),
        VariableValue::Float64(values) => fmu.setFloat64(&[value_reference], values),
        VariableValue::Int8(values) => fmu.setInt8(&[value_reference], values),
        VariableValue::UInt8(values) => fmu.setUInt8(&[value_reference], values),
        VariableValue::Int16(values) => fmu.setInt16(&[value_reference], values),
        VariableValue::UInt16(values) => fmu.setUInt16(&[value_reference], values),
        VariableValue::Int32(values) => fmu.setInt32(&[value_reference], values),
        VariableValue::UInt32(values) => fmu.setUInt32(&[value_reference], values),
        VariableValue::Int64(values) => fmu.setInt64(&[value_reference], values),
        VariableValue::UInt64(values) => fmu.setUInt64(&[value_reference], values),
        VariableValue::Boolean(values) => fmu.setBoolean(&[value_reference], values),
        VariableValue::String(values) => {
            let string_refs: Vec<&str> = values.iter().map(|x| x.as_str()).collect();
            fmu.setString(&[value_reference], &string_refs)
        }
        VariableValue::Binary(values) => {
            let values = values.iter().map(|x| x.as_slice()).collect::<Vec<_>>();
            fmu.setBinary(&[value_reference], values.as_slice())
        }
    }
}

pub fn call(status: fmi3Status) -> Result<fmi3Status, SimulationError> {
    if matches!(status, fmi3Status::Ok | fmi3Status::Warning) {
        Ok(status)
    } else {
        Err(SimulationError::FMICall)
    }
}

fn set_start_values(
    start_values: &Vec<(String, String)>,
    model_description: &ModelDescription,
    fmu: &FMU3,
) -> Result<fmi3Status, SimulationError> {
    let mut configuration_mode = false;

    let mut non_structural_start_values = vec![];

    // set structural parameters first
    for (var_name, literal) in start_values {
        let variable: &ModelVariable = model_description.variable_by_name(var_name)?;

        if variable.causality == Causality::StructuralParameter {
            if !configuration_mode {
                call(fmu.enterConfigurationMode())?;
                configuration_mode = true;
            }

            match parse_variable_value(&variable.variableType, literal) {
                Ok(value) => {
                    call(set_variable_value(fmu, variable.valueReference, &value))?;
                }
                Err(e) => {
                    return Err(SimulationError::Parameter(format!(
                        "Invalid value '{literal}' for variable '{var_name}': {e}"
                    )));
                }
            }
        } else {
            non_structural_start_values.push((var_name.clone(), literal.clone()));
        }
    }

    if configuration_mode {
        call(fmu.exitConfigurationMode())?;
    }

    // then the non-structural start values
    for (var_name, literal) in non_structural_start_values.iter() {
        let variable: &ModelVariable = model_description.variable_by_name(var_name)?;

        match parse_variable_value(&variable.variableType, literal) {
            Ok(value) => {
                call(set_variable_value(fmu, variable.valueReference, &value))?;
            }
            Err(e) => {
                return Err(SimulationError::Parameter(format!(
                    "Invalid value '{literal}' for variable '{var_name}': {e}"
                )));
            }
        }
    }

    Ok(fmi3Status::Ok)
}

fn read_initial_fmu_state(fmu: &FMU3, path: &Path) -> Result<(), SimulationError> {
    let serialized_state = fs::read(path).map_err(SimulationError::io(&path))?;
    let mut fmu_state = ptr::null_mut();

    call(fmu.deserializeFMUState(&serialized_state, &mut fmu_state))?;
    call(fmu.setFMUState(fmu_state))?;

    Ok(())
}

fn write_final_fmu_state(fmu: &FMU3, path: &Path) -> Result<(), SimulationError> {
    let mut fmu_state = ptr::null_mut();
    call(fmu.getFMUState(&mut fmu_state))?;

    let mut size = 0usize;
    call(fmu.serializedFMUStateSize(fmu_state, &mut size))?;

    let mut serialized_state = vec![0; size];
    call(fmu.serializeFMUState(fmu_state, &mut serialized_state))?;

    fs::write(path, &serialized_state).map_err(SimulationError::io(&path))?;

    Ok(())
}
