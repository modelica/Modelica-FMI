#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals, unused)]
use std::{ffi::{CStr, c_char, c_void}, path::Path, sync::{Arc, Mutex}};
use fmi::fmi2::FMU2;
use fmi::SHARED_LIBRARY_EXTENSION;
use fmi::fmi2::types::{fmi2OK, fmi2Warning, fmi2Error, fmi2Type::fmi2CoSimulation};
use url::Url;
use fmi::types::fmiStatus::{fmiOK, fmiWarning, fmiError};

struct FMUInstance<'a> {
    fmu: Option<FMU2<'a>>,
    
    infoMessages: Arc<Mutex<Vec<String>>>,
    infoMessageBuffer: Arc<Mutex<Vec<u8>>>,

    warningMessages: Arc<Mutex<Vec<String>>>,
    warningMessageBuffer: Arc<Mutex<Vec<u8>>>,
    
    errorMessages: Arc<Mutex<Vec<String>>>,
    errorMessageBuffer: Arc<Mutex<Vec<u8>>>,
}

macro_rules! get_instance {
    ($instance:expr) => {{
        if $instance.is_null() {
            return;
        }
        unsafe { &*($instance as *const FMUInstance) }
    }};
}

macro_rules! get_instance_mut {
    ($instance:expr) => {{
        if $instance.is_null() {
            return;
        }
        unsafe { &mut *($instance as *mut FMUInstance) }
    }};
}

macro_rules! get_fmu {
    ($instance:expr) => {{
        match $instance.fmu.as_ref() {
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

// set an error message if it has not been set yet by the FMU
macro_rules! call {
    ($instance:expr, $status:expr) => {
        if !matches!($status, fmi2OK | fmi2Warning) {
            let mut guard = $instance.errorMessages.lock().unwrap();
            if !guard.is_empty() {   
                guard.push("FMI call failed.".to_string());
            }
        }
    };
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_Create() -> *mut c_void {

    let instance = FMUInstance {
        fmu: None,
        infoMessages: Arc::new(Mutex::new(Vec::new())),
        infoMessageBuffer: Arc::new(Mutex::new(Vec::new())),
        warningMessages: Arc::new(Mutex::new(Vec::new())),
        warningMessageBuffer: Arc::new(Mutex::new(Vec::new())),
        errorMessages: Arc::new(Mutex::new(Vec::new())),
        errorMessageBuffer: Arc::new(Mutex::new(Vec::new())),
    };

    Box::into_raw(Box::new(instance)) as *mut c_void
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_Free(instance: *mut c_void) {

    if instance.is_null() {
        return;
    }
    
    let instance = unsafe { Box::from_raw(instance as *mut FMUInstance) };

    let mut fmu = instance.fmu.unwrap();

    fmu.terminate();
    fmu.freeInstance();
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_Load(
    instance: *mut c_void,
    unzipdir: *const c_char,
    fmiVersion: i32,
    modelIdentifier: *const c_char,
    instanceName: *const c_char,
    interfaceType: i32,
    instantiationToken: *const c_char,
    visible: i32,
    loggingOn: i32,
    logFMICalls: i32,
    logToFile: i32,
    logFile: *const c_char,
    copyPlatformBinary: i32,
) {
    let instance: &mut FMUInstance<'_> = unsafe { &mut *(instance as *mut FMUInstance) };

    let unzipdir = unsafe { std::ffi::CStr::from_ptr(unzipdir) };
    let unzipdir = Path::new(unzipdir.to_str().unwrap());

    let modelIdentifier = unsafe { std::ffi::CStr::from_ptr(modelIdentifier) };
    let modelIdentifier = modelIdentifier.to_str().unwrap();

    let share_library_filename = format!("{}{}", modelIdentifier, SHARED_LIBRARY_EXTENSION);
    let path = unzipdir.join("binaries").join("win64").join(share_library_filename);

    let instanceName = unsafe { std::ffi::CStr::from_ptr(instanceName) };
    let instanceName = instanceName.to_str().unwrap();

    let mut info_messages = instance.infoMessages.clone();

    let log_fmi_call = move |status: &fmi::types::fmiStatus, message: &str| {        
        info_messages.lock().unwrap().push(message.to_string());
    };

    let mut info_messages = instance.infoMessages.clone();
    let mut warning_messages = instance.warningMessages.clone();
    let mut error_messages = instance.errorMessages.clone();

    let log_message = move |status: &fmi::types::fmiStatus, category: &str, message: &str| {

        let messages = match status {
            fmiOK => &info_messages,
            fmiWarning => &warning_messages,
            _ => &error_messages,
        };

        messages.lock().unwrap().push(message.to_string());
    };

    let mut fmu = FMU2::new(
        path.as_path(),
        instanceName,
        if logFMICalls != 0 { Some(Box::new(log_fmi_call)) } else { None },
        Some(Box::new(log_message))
    ).unwrap();

    let guid = unsafe { std::ffi::CStr::from_ptr(instantiationToken) };
    let guid = guid.to_str().unwrap();

    let resources_path = unzipdir.join("resources").join("");

    let resourceUrl = if resources_path.is_dir() {
        Some(Url::from_directory_path(&resources_path).unwrap())
    } else {
        None
    };

    let visible = visible != 0;
    let loggingOn = loggingOn != 0;

    let interfaceType = match interfaceType {
        0 => fmi::fmi2::types::fmi2Type::fmi2ModelExchange,
        1 => fmi::fmi2::types::fmi2Type::fmi2CoSimulation,
        _ => {
            let mut guard = instance.errorMessages.lock().unwrap();
            guard.push("Invalid interface type.".to_string());
            return;
        },
    };

    fmu.instantiate(instanceName, interfaceType, guid, resourceUrl.as_ref(), visible, loggingOn);

    instance.fmu = Some(fmu);
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_getInfoMessage(instance: *mut c_void) -> *const c_char {

    if instance.is_null() {
        return "\0" as *const str as *const c_char;
    }

    let instance = unsafe { &mut *(instance as *mut FMUInstance) };

    let mut buffer = instance.infoMessageBuffer.lock().unwrap();
    buffer.clear();

    let mut messages = instance.infoMessages.lock().unwrap();

    if let Some(message) = messages.pop() {
        buffer.extend_from_slice(message.as_bytes());
    }

    buffer.push(0); // null-terminate

    buffer.as_ptr() as *const c_char
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_getWarningMessage(instance: *mut c_void) -> *const c_char {

    if instance.is_null() {
        return "\0" as *const str as *const c_char;
    }

    let instance = unsafe { &mut *(instance as *mut FMUInstance) };

    let mut buffer = instance.warningMessageBuffer.lock().unwrap();
    buffer.clear();

    let mut messages = instance.warningMessages.lock().unwrap();

    if let Some(message) = messages.pop() {
        buffer.extend_from_slice(message.as_bytes());
    }

    buffer.push(0); // null-terminate

    buffer.as_ptr() as *const c_char
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_getErrorMessage(instance: *mut c_void) -> *const c_char {
    
    if instance.is_null() {
        return "\0" as *const str as *const c_char;
    }

    let instance = unsafe { &mut *(instance as *mut FMUInstance) };

    let mut buffer = instance.errorMessageBuffer.lock().unwrap();
    buffer.clear();

    let mut messages = instance.errorMessages.lock().unwrap();

    for message in messages.drain(..) {
        buffer.extend_from_slice(message.as_bytes());
        buffer.push(b'\n');
    }

    buffer.push(0); // null-terminate

    buffer.as_ptr() as *const c_char
}

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
