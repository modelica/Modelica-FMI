#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals, unused)]
use std::{ffi::{c_char, c_void}, path::Path, sync::{Arc, Mutex}};
use fmi::fmi2::FMU2;
use fmi::SHARED_LIBRARY_EXTENSION;
use fmi::fmi2::types::{fmi2OK, fmi2Warning, fmi2Error};

struct FMUInstance<'a> {
    fmu: Option<FMU2<'a>>,
    errorMessage: Arc<Mutex<Vec<u8>>>,
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_Create() -> *mut c_void {

    let instance = FMUInstance {
        fmu: None,
        errorMessage: Arc::new(Mutex::new(Vec::new())),
    };

    Box::into_raw(Box::new(instance)) as *mut c_void
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_Free(instance: *mut c_void) {

    if !instance.is_null() {
        let _ = unsafe { Box::from_raw(instance as *mut FMUInstance) };
    }
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
    let instance = unsafe { &mut *(instance as *mut FMUInstance) };

    let unzipdir = unsafe { std::ffi::CStr::from_ptr(unzipdir) };
    let unzipdir = Path::new(unzipdir.to_str().unwrap());

    let modelIdentifier = unsafe { std::ffi::CStr::from_ptr(modelIdentifier) };
    let modelIdentifier = modelIdentifier.to_str().unwrap();

    // let path = Path::new(r"E:\WS\Modelica-FMI\FMI\Resources\FMUs\1604856\binaries\win64\BouncingBall.dll");
    let share_library_filename = format!("{}{}", modelIdentifier, SHARED_LIBRARY_EXTENSION);
    let path = unzipdir.join("binaries").join("win64").join(share_library_filename);

    println!("Loading FMU from path: {:?}", path);

    let instanceName = unsafe { std::ffi::CStr::from_ptr(instanceName) };
    let instanceName = instanceName.to_str().unwrap();

    let log_fmi_call = |status: &fmi::types::fmiStatus, message: &str| {
        println!("[FMICall][{:?}] {}", status, message);
    };

    let error_msg = instance.errorMessage.clone();

    let log_message = move |status: &fmi::types::fmiStatus, category: &str, message: &str| {
        // println!("[Message][{:?}][{}] {}", status, category, message);
        // append message to instance.errorMessage (thread-safe)
        let full_message = format!("[{:?}][{}] {}\0", status, category, message);
        let mut guard = error_msg.lock().unwrap();
        guard.extend_from_slice(full_message.as_bytes());
    };

    let mut fmu = FMU2::new(
        path.as_path(),
        instanceName,
        Some(Box::new(log_fmi_call)), 
        Some(Box::new(log_message))
    ).unwrap();

    let instantiation_token = unsafe { std::ffi::CStr::from_ptr(instantiationToken) };
    let instantiation_token = instantiation_token.to_str().unwrap();

    fmu.instantiate(instanceName, fmi::fmi2::types::fmi2Type::fmi2CoSimulation, instantiation_token, None, false, false);

    instance.fmu = Some(fmu);
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_getInfoMessage(instance: *mut c_void) -> *const c_char {
    "\0" as *const str as *const c_char
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_getWarningMessage(instance: *mut c_void) -> *const c_char {
    "\0" as *const str as *const c_char
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_getErrorMessage(instance: *mut c_void) -> *const c_char {
    
    if instance.is_null() {
        return "\0" as *const str as *const c_char;
    }

    let instance = unsafe { &mut *(instance as *mut FMUInstance) };

    let guard = instance.errorMessage.lock().unwrap();
    if guard.is_empty() {
        "\0" as *const str as *const c_char
    } else {
        guard.as_ptr() as *const c_char
    }
}

/***************************************************
Common Functions
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetReal(instance: *mut c_void, vr: i32, value: *mut f64) {
    let instance = unsafe { &mut *(instance as *mut FMUInstance) };
    let fmu = instance.fmu.as_mut().unwrap();

    let valueReferences = [vr as u32];
    let values = unsafe { std::slice::from_raw_parts_mut(value as *mut f64, 1) };

    fmu.getReal(&valueReferences, values);
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetInteger(instance: *mut c_void, vr: i32, value: *mut i32) {

}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2GetBoolean(instance: *mut c_void, vr: i32, value: *mut i32) {
    
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetReal(instance: *mut c_void, vr: *const i32, nvr: i32, value: *const f64) {
    
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetInteger(instance: *mut c_void, vr: *const i32, nvr: i32, value: *const i32) {
    
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetBoolean(instance: *mut c_void, vr: *const i32, nvr: i32, value: *const i32) {
    
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetString(instance: *mut c_void, vr: *const i32, nvr: i32, value: *const *const c_char) {
    
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2SetupExperiment(instance: *mut c_void,
    toleranceDefined: i32,
    tolerance: f64,
    startTime: f64,
    stopTimeDefined: i32,
    stopTime: f64) {

    if instance.is_null() {
        return;
    }

    let instance = unsafe { &mut *(instance as *mut FMUInstance) };

    let fmu = instance.fmu.as_mut().unwrap();

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

    if !matches!(fmu.setupExperiment(tolerance, 10.0, stopTime), fmi2OK | fmi2Warning) {
        let mut guard = instance.errorMessage.lock().unwrap();
        guard.extend_from_slice(b"Error in setupExperiment\0");
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2EnterInitializationMode(instance: *mut c_void) {
    
    let instance = unsafe { &mut *(instance as *mut FMUInstance) };
    let fmu = instance.fmu.as_mut().unwrap();

    fmu.enterInitializationMode();
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2ExitInitializationMode(instance: *mut c_void) {

    let instance = unsafe { &mut *(instance as *mut FMUInstance) };
    let fmu = instance.fmu.as_mut().unwrap();

    fmu.exitInitializationMode();
}

/***************************************************
Model Exchange
****************************************************/

// EXPORT void FMU_FMI2EnterEventMode(instance: *mut c_void);

// EXPORT void FMU_FMI2NewDiscreteStates(instance: *mut c_void, int* valuesOfContinuousStatesChanged, double* nextEventTime);

// EXPORT void FMU_FMI2EnterContinuousTimeMode(instance: *mut c_void);

// EXPORT void FMU_FMI2SetTime(instance: *mut c_void, double time);

// EXPORT void FMU_FMI2SetContinuousStates(instance: *mut c_void, const double x[], int nx);

// EXPORT void FMU_FMI2GetDerivatives(instance: *mut c_void, double derivatives[], int nx);

// EXPORT void FMU_FMI2GetEventIndicators(instance: *mut c_void, double eventIndicators[], int ni);

// EXPORT void FMU_FMI2GetContinuousStates(instance: *mut c_void, double x[], int nx);

/***************************************************
Co-Simulation
****************************************************/

#[unsafe(no_mangle)]
pub extern "C" fn FMU_FMI2DoStep(instance: *mut c_void,
    currentCommunicationPoint: f64,
    communicationStepSize: f64,
    noSetFMUStatePriorToCurrentPoint: i32) {

    let instance = unsafe { &mut *(instance as *mut FMUInstance) };
    let fmu = instance.fmu.as_mut().unwrap();

    fmu.doStep(currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint);

    // if !matches!(fmu.doStep(currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint), fmi2OK | fmi2Warning) {
    //     let mut guard = instance.errorMessage.lock().unwrap();
    //     guard.extend_from_slice(b"Error in doStep\0");
    // }
}
