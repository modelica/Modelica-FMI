#![allow(
    non_camel_case_types,
    non_snake_case,
    non_upper_case_globals,
    unused,
    clippy::missing_safety_doc
)]

use crate::common::{FMU, FMUInstance};
use fmi_rs::fmi3::types::fmi3Status;
use std::fs::File;
use std::{
    ffi::{CStr, c_char, c_void},
    io::Write,
    path::Path,
    sync::{Arc, Mutex},
};
use url::Url;

macro_rules! get_fmu {
    ($instance:expr) => {{
        match &$instance.fmu {
            Some(FMU::FMI3(fmu)) => fmu,
            _ => {
                $instance.log_error("FMU is not instantiated.".to_string());
                return;
            }
        }
    }};
}

macro_rules! call {
    ($instance:expr, $status:expr) => {
        if !matches!($status, fmi3Status::Ok | fmi3Status::Warning) {
            $instance.log_error("FMI call failed.".to_string());
        }
    };
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
    stopTime: f64,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let tolerance = if toleranceDefined != 0 {
        Some(tolerance)
    } else {
        None
    };
    let stopTime = if stopTimeDefined != 0 {
        Some(stopTime)
    } else {
        None
    };

    call!(
        instance,
        fmu.enterInitializationMode(tolerance, startTime, stopTime)
    );
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3ExitInitializationMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    call!(instance, fmu.exitInitializationMode());
}

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
pub unsafe extern "C" fn FMU_FMI3GetFloat32(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut f64,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![0f32; nValues as usize];

    call!(instance, fmu.getFloat32(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as f64;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetFloat64(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut f64,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    call!(instance, fmu.getFloat64(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetInt8(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![0i8; nValues as usize];

    call!(instance, fmu.getInt8(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetUInt8(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![0u8; nValues as usize];

    call!(instance, fmu.getUInt8(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetInt16(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![0i16; nValues as usize];

    call!(instance, fmu.getInt16(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetUInt16(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![0u16; nValues as usize];

    call!(instance, fmu.getUInt16(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetInt32(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    call!(instance, fmu.getInt32(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetUInt32(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![0u32; nValues as usize];

    call!(instance, fmu.getUInt32(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetInt64(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![0i64; nValues as usize];

    call!(instance, fmu.getInt64(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetUInt64(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![0u64; nValues as usize];

    call!(instance, fmu.getUInt64(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetBoolean(
    instance: *mut c_void,
    valueReference: i32,
    values: *mut i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [valueReference as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(values, nValues as usize) };

    let mut buffer = vec![false; nValues as usize];

    call!(instance, fmu.getBoolean(&valueReferences, &mut buffer[..]));

    for (i, &v) in buffer.iter().enumerate() {
        values[i] = v as i32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetFloat32(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const f64,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![0f32; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as f32;
    }

    call!(instance, fmu.setFloat32(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetFloat64(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const f64,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    call!(instance, fmu.setFloat64(valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetInt8(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![0i8; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as i8;
    }

    call!(instance, fmu.setInt8(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetUInt8(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![0u8; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as u8;
    }

    call!(instance, fmu.setUInt8(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetInt16(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![0i16; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as i16;
    }

    call!(instance, fmu.setInt16(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetUInt16(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![0u16; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as u16;
    }

    call!(instance, fmu.setUInt16(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetInt32(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    call!(instance, fmu.setInt32(valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetUInt32(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![0u32; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as u32;
    }

    call!(instance, fmu.setUInt32(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetInt64(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![0i64; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as i64;
    }

    call!(instance, fmu.setInt64(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetUInt64(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![0u64; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v as u64;
    }

    call!(instance, fmu.setUInt64(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetBoolean(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const i32,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };

    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let mut buffer = vec![false; nValues as usize];

    for (i, &v) in values.iter().enumerate() {
        buffer[i] = v != 0;
    }

    call!(instance, fmu.setBoolean(valueReferences, &buffer[..]));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3SetString(
    instance: *mut c_void,
    valueReferences: *const i32,
    nValueReferences: i32,
    values: *const *const c_char,
    nValues: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe {
        std::slice::from_raw_parts(valueReferences as *const u32, nValueReferences as usize)
    };
    let values = unsafe { std::slice::from_raw_parts(values, nValues as usize) };

    let values: Vec<String> = values
        .iter()
        .map(|&v| unsafe { CStr::from_ptr(v).to_string_lossy().into_owned() })
        .collect();

    let v: Vec<&str> = values.iter().map(|v| v.as_str()).collect();

    call!(instance, fmu.setString(valueReferences, &v));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3UpdateDiscreteStates(
    instance: *mut c_void,
    valuesOfContinuousStatesChanged: *mut i32,
    nextEventTime: *mut f64,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let mut discrete_states_need_update = false;
    let mut terminate_simulation = false;
    let mut nominals_of_continuous_states_changed = false;
    let mut values_of_continuous_states_changed = false;
    let mut next_event_time = None;

    call!(
        instance,
        fmu.updateDiscreteStates(
            &mut discrete_states_need_update,
            &mut terminate_simulation,
            &mut nominals_of_continuous_states_changed,
            &mut values_of_continuous_states_changed,
            &mut next_event_time,
        )
    );

    if !valuesOfContinuousStatesChanged.is_null() {
        unsafe {
            *valuesOfContinuousStatesChanged = values_of_continuous_states_changed as i32;
        }
    }

    if let Some(next_event_time) = next_event_time
        && !nextEventTime.is_null()
    {
        unsafe { *nextEventTime = next_event_time };
    }
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
pub unsafe extern "C" fn FMU_FMI3SetContinuousStates(
    instance: *mut c_void,
    continuousStates: *const f64,
    nContinuousStates: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let continuousStates =
        unsafe { std::slice::from_raw_parts(continuousStates, nContinuousStates as usize) };
    call!(instance, fmu.setContinuousStates(continuousStates));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetContinuousStateDerivatives(
    instance: *mut c_void,
    derivatives: *mut f64,
    nContinuousStates: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let derivatives =
        unsafe { std::slice::from_raw_parts_mut(derivatives, nContinuousStates as usize) };
    call!(instance, fmu.getContinuousStateDerivatives(derivatives));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetEventIndicators(
    instance: *mut c_void,
    eventIndicators: *mut f64,
    nEventIndicators: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let eventIndicators =
        unsafe { std::slice::from_raw_parts_mut(eventIndicators, nEventIndicators as usize) };
    call!(instance, fmu.getEventIndicators(eventIndicators));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI3GetContinuousStates(
    instance: *mut c_void,
    continuousStates: *mut f64,
    nContinuousStates: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let continuousStates =
        unsafe { std::slice::from_raw_parts_mut(continuousStates, nContinuousStates as usize) };
    call!(instance, fmu.getContinuousStates(continuousStates));
}

/***************************************************
Functions for Co-Simulation
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI3DoStep(
    instance: *mut c_void,
    currentCommunicationPoint: f64,
    communicationStepSize: f64,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let mut eventHandlingNeeded = false;
    let mut terminateSimulation = false;
    let mut earlyReturn = false;
    let mut lastSuccessfulTime = 0.0;

    call!(
        instance,
        fmu.doStep(
            currentCommunicationPoint,
            communicationStepSize,
            false,
            &mut eventHandlingNeeded,
            &mut terminateSimulation,
            &mut earlyReturn,
            &mut lastSuccessfulTime
        )
    );
}
