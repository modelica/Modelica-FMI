#![allow(
    non_camel_case_types,
    non_snake_case,
    non_upper_case_globals,
    unused,
    clippy::missing_safety_doc
)]

use fmi_rs::{
    SHARED_LIBRARY_EXTENSION,
    fmi2::{CS, FMU2, PLATFORM, log, types::fmi2Status},
    fmi3::{FMU3, types::fmi3Status},
};
use std::{cell::RefCell, fs::File};
use std::{
    ffi::{CStr, c_char, c_void},
    io::Write,
    path::Path,
    sync::{Arc, Mutex},
};
use url::Url;

pub enum FMU {
    FMI2(Arc<FMU2<CS>>),
    FMI3(Arc<FMU3>),
}

pub struct FMUInstance {
    pub fmu: Option<FMU>,
    log_file: Option<RefCell<File>>,
    info_messages: RefCell<Vec<String>>,
    warning_messages: RefCell<Vec<String>>,
    error_messages: RefCell<Vec<String>>,
    message_buffer: RefCell<Vec<u8>>,
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
                return;
            }
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

// pub struct MyLogger;

impl FMUInstance {
    pub fn log_call(&self, message: &str) {
        self.info_messages
            .borrow_mut()
            .push(format!("[FMI] {message}"));
    }

    pub fn log_info(&self, message: String) {
        self.info_messages.borrow_mut().push(message);
    }

    pub fn log_warning(&self, message: String) {
        self.warning_messages.borrow_mut().push(message);
    }

    pub fn log_error(&self, message: String) {
        self.error_messages.borrow_mut().push(message);
    }
}

impl fmi_rs::fmi2::log::Logger for FMUInstance {
    fn log_call(&self, _status: fmi2Status, message: &str) {
        self.log_call(message);
    }

    fn log_message(&self, status: fmi2Status, category: &str, message: &str) {
        let message = format!("[{category}] {message}");
        match status {
            fmi2Status::Ok => self.log_info(message),
            fmi2Status::Warning => self.log_warning(message),
            _ => self.log_error(message),
        }
    }
}

impl fmi_rs::fmi3::log::Logger for FMUInstance {
    fn log_call(&self, _status: fmi3Status, message: &str) {
        self.log_call(message);
    }

    fn log_message(&self, status: fmi3Status, category: &str, message: &str) {
        let message = format!("[{category}] {message}");
        match status {
            fmi3Status::Ok => self.log_info(message),
            fmi3Status::Warning => self.log_warning(message),
            _ => self.log_error(message),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_Create() -> *mut c_void {
    #[allow(clippy::arc_with_non_send_sync)]
    let instance = Arc::new(FMUInstance {
        fmu: None,
        log_file: None,
        info_messages: RefCell::new(Vec::new()),
        warning_messages: RefCell::new(Vec::new()),
        error_messages: RefCell::new(Vec::new()),
        message_buffer: RefCell::new(Vec::new()),
    });

    Arc::into_raw(instance) as *mut c_void
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_Free(instance: *mut c_void) {
    if instance.is_null() {
        return;
    }

    let instance = unsafe { Arc::from_raw(instance as *mut FMUInstance) };

    // let instance = unsafe { Box::from_raw(instance as *mut FMUInstance) };

    match &instance.fmu {
        Some(FMU::FMI2(fmu)) => {
            fmu.terminate();
        }
        Some(FMU::FMI3(fmu)) => {
            fmu.terminate();
        }
        _ => (),
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn FMU_Load(
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
    let instance: &mut FMUInstance = unsafe { &mut *(instance as *mut FMUInstance) };

    let unzipdir = unsafe { std::ffi::CStr::from_ptr(unzipdir) };
    let unzipdir = Path::new(unzipdir.to_str().unwrap());

    let modelIdentifier = unsafe { std::ffi::CStr::from_ptr(modelIdentifier) };
    let modelIdentifier = modelIdentifier.to_str().unwrap();

    let share_library_filename = format!("{}{}", modelIdentifier, SHARED_LIBRARY_EXTENSION);

    let instanceName = unsafe { std::ffi::CStr::from_ptr(instanceName) };
    let instanceName = instanceName.to_str().unwrap();

    instance.log_file = if logToFile != 0 {
        let log_file_cstr = unsafe { std::ffi::CStr::from_ptr(logFile) };
        let log_file_str = log_file_cstr.to_str().unwrap();
        let mut log_file = File::create(log_file_str).unwrap();
        Some(RefCell::new(log_file))
    } else {
        None
    };

    let visible = visible != 0;
    let loggingOn = loggingOn != 0;
    let resources_path = unzipdir.join("resources").join("");
    let guid = unsafe { std::ffi::CStr::from_ptr(instantiationToken) };
    let guid = guid.to_str().unwrap();
    let logCalls = logFMICalls != 0;

    let instance_arc: Arc<FMUInstance> = unsafe { Arc::from_raw(instance as *mut FMUInstance) };

    if fmiVersion == 2 {
        let logger: Arc<dyn fmi_rs::fmi2::log::Logger> = instance_arc.clone();

        let mut fmu = FMU2::<CS>::new(
            unzipdir,
            modelIdentifier,
            instanceName,
            guid,
            visible,
            loggingOn,
            logCalls,
            logger,
            true,
        )
        .unwrap();

        instance.fmu = Some(FMU::FMI2(fmu));
    } else if fmiVersion == 3 {
        let logger: Arc<dyn fmi_rs::fmi3::log::Logger> = instance_arc.clone();

        let fmu = FMU3::instantiateCoSimulation(
            unzipdir,
            modelIdentifier,
            instanceName,
            guid,
            visible,
            loggingOn,
            false,
            false,
            logger,
            logCalls,
            None,
        )
        .unwrap();

        instance.fmu = Some(FMU::FMI3(fmu));
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn FMU_getInfoMessage(instance: *mut c_void) -> *const c_char {
    if instance.is_null() {
        return "\0" as *const str as *const c_char;
    }

    let instance = unsafe { &mut *(instance as *mut FMUInstance) };

    let mut buffer = instance.message_buffer.borrow_mut();

    buffer.clear();

    if let Some(message) = instance.info_messages.borrow_mut().pop() {
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

    let mut buffer = instance.message_buffer.borrow_mut();

    buffer.clear();

    if let Some(message) = instance.warning_messages.borrow_mut().pop() {
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

    let mut buffer = instance.message_buffer.borrow_mut();

    buffer.clear();

    for message in instance.error_messages.borrow_mut().drain(..) {
        buffer.extend_from_slice(message.as_bytes());
        buffer.push(b'\n');
    }

    buffer.push(0); // null-terminate

    buffer.as_ptr() as *const c_char
}
