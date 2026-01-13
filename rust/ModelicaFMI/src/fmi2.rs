#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals, unused)]
use std::{ffi::{CStr, c_char, c_void}, io::Write, path::Path, sync::{Arc, Mutex}};
use fmi::fmi2::FMU2;
use fmi::SHARED_LIBRARY_EXTENSION;
use fmi::fmi2::types::{fmi2OK, fmi2Warning, fmi2Error, fmi2Type::fmi2CoSimulation};
use url::Url;
use fmi::types::fmiStatus::{fmiOK, fmiWarning, fmiError};
use std::fs::File;
use crate::common::FMUInstance;

// macro_rules! get_instance {
//     ($instance:expr) => {{
//         if $instance.is_null() {
//             return;
//         }
//         unsafe { &*($instance as *const FMUInstance) }
//     }};
// }

// macro_rules! get_instance_mut {
//     ($instance:expr) => {{
//         if $instance.is_null() {
//             return;
//         }
//         unsafe { &mut *($instance as *mut FMUInstance) }
//     }};
// }

macro_rules! get_fmu {
    ($instance:expr) => {{
        match $instance.fmu2.as_ref() {
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

// // set an error message if it has not been set yet by the FMU
// macro_rules! call {
//     ($instance:expr, $status:expr) => {
//         if !matches!($status, fmi2OK | fmi2Warning) {
//             let mut guard = $instance.errorMessages.lock().unwrap();
//             if !guard.is_empty() {   
//                 guard.push("FMI call failed.".to_string());
//             }
//         }
//     };
// }

/***************************************************
Common Functions
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetReal(instance: *mut c_void, vr: i32, value: *mut f64) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [vr as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(value as *mut f64, 1) };
    
    call!(instance, fmu.getReal(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetInteger(instance: *mut c_void, vr: i32, value: *mut i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [vr as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(value as *mut i32, 1) };
    
    call!(instance, fmu.getInteger(&valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetBoolean(instance: *mut c_void, vr: i32, value: *mut i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = [vr as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(value as *mut i32, 1) };
    
    call!(instance, fmu.getBoolean(&valueReferences, values)); 
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetReal(instance: *mut c_void, vr: *const i32, nvr: i32, value: *const f64) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(vr as *const u32, nvr as usize) };
    let values = unsafe { std::slice::from_raw_parts(value as *const f64, nvr as usize) };
    
    call!(instance, fmu.setReal(valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetInteger(instance: *mut c_void, vr: *const i32, nvr: i32, value: *const i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(vr as *const u32, nvr as usize) };
    let values = unsafe { std::slice::from_raw_parts(value as *const i32, nvr as usize) };

    call!(instance, fmu.setInteger(valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetBoolean(instance: *mut c_void, vr: *const i32, nvr: i32, value: *const i32) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    
    let valueReferences = unsafe { std::slice::from_raw_parts(vr as *const u32, nvr as usize) };
    let values = unsafe { std::slice::from_raw_parts(value as *const i32, nvr as usize) };

    call!(instance, fmu.setBoolean(valueReferences, values));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetString(instance: *mut c_void, vr: *const i32, nvr: i32, value: *const *const c_char) {
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
pub extern "C" fn FMU_FMI2SetupExperiment(instance: *mut c_void,
    toleranceDefined: i32,
    tolerance: f64,
    startTime: f64,
    stopTimeDefined: i32,
    stopTime: f64) {

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

    call!(instance, fmu.setupExperiment(tolerance, startTime, stopTime));
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2EnterInitializationMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    call!(instance, fmu.enterInitializationMode());
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2ExitInitializationMode(instance: *mut c_void) {
    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);
    call!(instance, fmu.exitInitializationMode());
}

/***************************************************
Model Exchange
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2EnterEventMode(instance: *mut c_void) {
    todo!()
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2NewDiscreteStates(instance: *mut c_void, valuesOfContinuousStatesChanged: *mut i32, nextEventTime: *mut f64) {
    todo!()
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2EnterContinuousTimeMode(instance: *mut c_void) {
    todo!()
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetTime(instance: *mut c_void, time: f64) {
    todo!()
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetContinuousStates(instance: *mut c_void, x: *const f64, nx: i32) {
    todo!()
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetDerivatives(instance: *mut c_void, derivatives: *mut f64, nx: i32) {
    todo!()
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetEventIndicators(instance: *mut c_void, eventIndicators: *mut f64, ni: i32) {
    todo!()
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetContinuousStates(instance: *mut c_void, x: *mut f64, nx: i32) {
    todo!()
}

/***************************************************
Co-Simulation
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2DoStep(instance: *mut c_void,
    currentCommunicationPoint: f64,
    communicationStepSize: f64,
    noSetFMUStatePriorToCurrentPoint: i32) {

    let instance = get_instance!(instance);
    let fmu = get_fmu!(instance);

    call!(instance, fmu.doStep(currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint));
}
