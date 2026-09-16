#![allow(
    non_camel_case_types,
    non_snake_case,
    non_upper_case_globals,
    unused,
    clippy::missing_safety_doc
)]

use crate::common::{FMU, FMUInstance};
use fmi_rs::fmi2::types::fmi2Status;
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
            Some(FMU::FMI2(fmu)) => fmu,
            _ => {
                $instance.log_error("FMU is not instantiated.".to_string());
                return;
            }
        }
    }};
}

macro_rules! call {
    ($instance:expr, $status:expr) => {
        if !matches!($status, fmi2Status::Ok | fmi2Status::Warning) {
            $instance.log_error("FMI call failed.".to_string());
        }
    };
}

/***************************************************
Common Functions
****************************************************/

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2GetReal(instance: *mut c_void, vr: i32, value: *mut f64) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [vr as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(value, 1) };

    call!(instance, fmu.getReal(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2GetInteger(instance: *mut c_void, vr: i32, value: *mut i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [vr as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(value, 1) };

    call!(instance, fmu.getInteger(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2GetBoolean(instance: *mut c_void, vr: i32, value: *mut i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = [vr as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(value, 1) };

    call!(instance, fmu.getBoolean(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2SetReal(
    instance: *mut c_void,
    vr: *const i32,
    nvr: i32,
    value: *const f64,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe { std::slice::from_raw_parts(vr as *const u32, nvr as usize) };
    let values = unsafe { std::slice::from_raw_parts(value, nvr as usize) };

    call!(instance, fmu.setReal(valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2SetInteger(
    instance: *mut c_void,
    vr: *const i32,
    nvr: i32,
    value: *const i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe { std::slice::from_raw_parts(vr as *const u32, nvr as usize) };
    let values = unsafe { std::slice::from_raw_parts(value, nvr as usize) };

    call!(instance, fmu.setInteger(valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2SetBoolean(
    instance: *mut c_void,
    vr: *const i32,
    nvr: i32,
    value: *const i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe { std::slice::from_raw_parts(vr as *const u32, nvr as usize) };
    let values = unsafe { std::slice::from_raw_parts(value, nvr as usize) };

    call!(instance, fmu.setBoolean(valueReferences, values));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2SetString(
    instance: *mut c_void,
    vr: *const i32,
    nvr: i32,
    value: *const *const c_char,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    let valueReferences = unsafe { std::slice::from_raw_parts(vr as *const u32, nvr as usize) };
    let values = unsafe { std::slice::from_raw_parts(value, nvr as usize) };

    let values: Vec<String> = values
        .iter()
        .map(|&v| unsafe { CStr::from_ptr(v).to_string_lossy().into_owned() })
        .collect();

    let v: Vec<&str> = values.iter().map(|v| v.as_str()).collect();

    call!(instance, fmu.setString(valueReferences, &v));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2SetupExperiment(
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
        fmu.setupExperiment(tolerance, startTime, stopTime)
    );
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2EnterInitializationMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    call!(instance, fmu.enterInitializationMode());
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2ExitInitializationMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    call!(instance, fmu.exitInitializationMode());
}

/***************************************************
Model Exchange
****************************************************/

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2EnterEventMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    todo!()
    // call!(instance, fmu.enterEventMode());
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2NewDiscreteStates(
    instance: *mut c_void,
    valuesOfContinuousStatesChanged: *mut i32,
    nextEventTime: *mut f64,
) {
    todo!()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2EnterContinuousTimeMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    todo!()
    // call!(instance, fmu.enterContinuousTimeMode());
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2SetTime(instance: *mut c_void, time: f64) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    // call!(instance, fmu.setTime(time));
    todo!()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2SetContinuousStates(
    instance: *mut c_void,
    x: *const f64,
    nx: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let x = unsafe { std::slice::from_raw_parts(x, nx as usize) };
    // call!(instance, fmu.setContinuousStates(x));
    todo!()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2GetDerivatives(
    instance: *mut c_void,
    derivatives: *mut f64,
    nx: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let derivatives = unsafe { std::slice::from_raw_parts_mut(derivatives, nx as usize) };
    // call!(instance, fmu.getDerivatives(derivatives));
    todo!()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2GetEventIndicators(
    instance: *mut c_void,
    eventIndicators: *mut f64,
    ni: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let eventIndicators = unsafe { std::slice::from_raw_parts_mut(eventIndicators, ni as usize) };
    // call!(instance, fmu.getEventIndicators(eventIndicators));
    todo!()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2GetContinuousStates(instance: *mut c_void, x: *mut f64, nx: i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    let x = unsafe { std::slice::from_raw_parts_mut(x, nx as usize) };
    // call!(instance, fmu.getContinuousStates(x));
    todo!()
}

/***************************************************
Co-Simulation
****************************************************/

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_FMI2DoStep(
    instance: *mut c_void,
    currentCommunicationPoint: f64,
    communicationStepSize: f64,
    noSetFMUStatePriorToCurrentPoint: i32,
) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    call!(
        instance,
        fmu.doStep(
            currentCommunicationPoint,
            communicationStepSize,
            noSetFMUStatePriorToCurrentPoint
        )
    );
}
