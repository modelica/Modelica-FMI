#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals, unused)]
use std::{ffi::{CStr, c_char, c_void}, io::Write, path::Path, sync::{Arc, Mutex}};
use fmi::fmi2::FMU2;
use fmi::SHARED_LIBRARY_EXTENSION;
use fmi::fmi2::types::{fmi2OK, fmi2Warning, fmi2Error, fmi2Type::fmi2CoSimulation};
use url::Url;
use fmi::types::fmiStatus::{fmiOK, fmiWarning, fmiError};
use std::fs::File;
use crate::common::FMUInstance;

macro_rules! get_fmu {
    ($instance:expr) => {{
        match $instance.fmu3.as_ref() {
        Some(fmu) => fmu,
        None => {
            let mut guard = $instance.errorMessages.lock().unwrap();
            if !guard.is_empty() {   
                guard.push("FMU is not instantiated.".to_string());
            }
            return
        },
        }
    }};
}

/***************************************************
Common Functions
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3EnterInitializationMode(
    instance: *mut c_void,
    toleranceDefined: i32,
    tolerance: f64,
    startTime: f64,
    stopTimeDefined: i32,
    stopTime: f64) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let tolerance = if toleranceDefined != 0 { Some(tolerance) } else { None };
    let stopTime = if stopTimeDefined != 0 { Some(stopTime) } else { None };

    call!(instance, fmu.enterInitializationMode(tolerance, startTime, stopTime));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3ExitInitializationMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);   
    call!(instance, fmu.exitInitializationMode());}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3EnterEventMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);   
    call!(instance, fmu.enterEventMode());
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3EnterConfigurationMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);   
    call!(instance, fmu.enterConfigurationMode());
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3ExitConfigurationMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);   
    call!(instance, fmu.exitConfigurationMode());
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetFloat32(instance: *mut c_void, valueReference: i32, values: *mut f64, nValues: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut f64, nValues as usize) };
    
    let mut buffer = vec![0f32; nValues as usize];

    call!(instance, fmu.getFloat32(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as f64;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetFloat64(instance: *mut c_void, valueReference: i32, values: *mut f64, nValues: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut f64, nValues as usize) };
    
    call!(instance, fmu.getFloat64(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetInt8(instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    let mut buffer = vec![0i8; nValues as usize];

    call!(instance, fmu.getInt8(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetUInt8(instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    let mut buffer = vec![0u8; nValues as usize];

    call!(instance, fmu.getUInt8(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetInt16(instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    let mut buffer = vec![0i16; nValues as usize];

    call!(instance, fmu.getInt16(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetUInt16( instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    let mut buffer = vec![0u16; nValues as usize];

    call!(instance, fmu.getUInt16(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetInt32(instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    call!(instance, fmu.getInt32(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetUInt32(instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    let mut buffer = vec![0u32; nValues as usize];

    call!(instance, fmu.getUInt32(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetInt64(instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    let mut buffer = vec![0i64; nValues as usize];

    call!(instance, fmu.getInt64(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetUInt64(instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    let mut buffer = vec![0u64; nValues as usize];

    call!(instance, fmu.getUInt64(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetBoolean(instance: *mut c_void, valueReference: i32, values: *mut i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values as *mut i32, nValues as usize) };
    
    let mut buffer = vec![false; nValues as usize];

    call!(instance, fmu.getBoolean(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetFloat32(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const f64, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    let mut buffer = vec![0f32; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as f32;
    }
    
    call!(instance, fmu.setFloat32(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetFloat64(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const f64, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    call!(instance, fmu.setFloat64(valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetInt8(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    let mut buffer = vec![0i8; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as i8;
    }
    
    call!(instance, fmu.setInt8(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetUInt8(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    let mut buffer = vec![0u8; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as u8;
    }
    
    call!(instance, fmu.setUInt8(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetInt16(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    let mut buffer = vec![0i16; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as i16;
    }
    
    call!(instance, fmu.setInt16(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetUInt16(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    let mut buffer = vec![0u16; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as u16;
    }
    
    call!(instance, fmu.setUInt16(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetInt32(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    call!(instance, fmu.setInt32(valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetUInt32(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    let mut buffer = vec![0u32; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as u32;
    }
    
    call!(instance, fmu.setUInt32(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetInt64(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    let mut buffer = vec![0i64; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as i64;
    }
    
    call!(instance, fmu.setInt64(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetUInt64(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };
    
    let mut buffer = vec![0u64; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as u64;
    }
    
    call!(instance, fmu.setUInt64(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetBoolean(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const i32, nValues: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![false; nValues as usize];
    
    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v != 0;
    }

    call!(instance, fmu.setBoolean(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetString(instance: *mut c_void, valueReferences: *const i32, nValueReferences: i32, values: *const *const c_char, nValues: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize) };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let values: Vec<String> = values
                .iter()
                .map(|&v| unsafe { CStr::from_ptr(v).to_string_lossy().into_owned() })
                .collect();

    let v: Vec<&str> = values.iter().map(|v| v.as_str()).collect();

    call!(instance, fmu.setString(valueReferences, &v));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3UpdateDiscreteStates(instance: *mut c_void, valuesOfContinuousStatesChanged: *mut i32, nextEventTime: *mut f64) {
    
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let (
        discreteStatesNeedUpdate,
        terminateSimulation,
        nominalsOfContinuousStatesChanged,
        _valuesOfContinuousStatesChanged,
        _nextEventTime,
        status,
    ) = fmu.updateDiscreteStates();

    call!(instance, status);

    unsafe { *valuesOfContinuousStatesChanged = if _valuesOfContinuousStatesChanged { 1 } else { 0 }; }
    unsafe { *nextEventTime = _nextEventTime };
}

/***************************************************
Functions for Model Exchange
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3EnterContinuousTimeMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);   
    call!(instance, fmu.enterContinuousTimeMode());
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetTime(instance: *mut c_void, time: f64) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);   
    call!(instance, fmu.setTime(time));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3SetContinuousStates(instance: *mut c_void, continuousStates: *const f64, nContinuousStates: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let continuousStates = unsafe { std::slice::from_raw_parts(continuousStates as *const f64, nContinuousStates as usize) };
    call!(instance, fmu.setContinuousStates(continuousStates));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetContinuousStateDerivatives(instance: *mut c_void, derivatives: *mut f64, nContinuousStates: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let derivatives = unsafe { std::slice::from_raw_parts_mut(derivatives as *mut f64, nContinuousStates as usize) };
    call!(instance, fmu.getContinuousStateDerivatives(derivatives));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetEventIndicators(instance: *mut c_void, eventIndicators: *mut f64, nEventIndicators: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let eventIndicators = unsafe { std::slice::from_raw_parts_mut(eventIndicators as *mut f64, nEventIndicators as usize) };
    call!(instance, fmu.getEventIndicators(eventIndicators));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3GetContinuousStates(instance: *mut c_void, continuousStates: *mut f64, nContinuousStates: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let continuousStates = unsafe { std::slice::from_raw_parts_mut(continuousStates as *mut f64, nContinuousStates as usize) };
    call!(instance, fmu.getContinuousStates(continuousStates));
}

/***************************************************
Functions for Co-Simulation
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3DoStep(instance: *mut c_void, currentCommunicationPoint: f64, communicationStepSize: f64) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let mut eventHandlingNeeded = false;
    let mut terminateSimulation = false;
    let mut earlyReturn = false;
    let mut lastSuccessfulTime = 0.0;

    call!(instance, fmu.doStep(currentCommunicationPoint, communicationStepSize, false, &mut eventHandlingNeeded, &mut terminateSimulation, &mut earlyReturn, &mut lastSuccessfulTime));
}