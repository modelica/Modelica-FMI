#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals, unused)]
use std::{ffi::{CStr, c_char, c_void}, io::Write, path::Path, sync::{Arc, Mutex}};
use fmi::{fmi2::FMU2, fmi3::FMU3};
use fmi::SHARED_LIBRARY_EXTENSION;
use fmi::fmi2::types::{fmi2OK, fmi2Warning, fmi2Error, fmi2Type::fmi2CoSimulation};
use url::Url;
use fmi::types::fmiStatus::{fmiOK, fmiWarning, fmiError};
use std::fs::File;

pub struct FMUInstance<'a> {
    pub fmu: Option<FMU2<'a>>,
    pub fmu3: Option<FMU3<'a>>,
    
    pub infoMessages: Arc<Mutex<Vec<String>>>,
    pub infoMessageBuffer: Arc<Mutex<Vec<u8>>>,

    pub warningMessages: Arc<Mutex<Vec<String>>>,
    pub warningMessageBuffer: Arc<Mutex<Vec<u8>>>,

    pub errorMessages: Arc<Mutex<Vec<String>>>,
    pub errorMessageBuffer: Arc<Mutex<Vec<u8>>>,
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
        fmu3: None,
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

    let log_file_option = if logToFile != 0 {
        let log_file_cstr = unsafe { std::ffi::CStr::from_ptr(logFile) };
        let log_file_str = log_file_cstr.to_str().unwrap();
        let mut log_file = File::create(log_file_str).unwrap();
        let mut log_file_ref = Arc::new(Mutex::new(log_file));
        Some(log_file_ref)
    } else {
        None
    };
    
    let log_fmi_call = move |status: &fmi::types::fmiStatus, message: &str| {
        if let Some(log_file_ref) = &log_file_option {
            let mut log_file = log_file_ref.lock().unwrap();
            log_file.write_all(message.as_bytes()).unwrap();
            log_file.write_all(b"\n").unwrap();
        } else {
            let mut messages = info_messages.lock().unwrap();
            messages.push(message.to_string());
        }
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