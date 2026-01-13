#![allow(non_snake_case)]

macro_rules! get_instance {
    ($instance:expr) => {{
        if $instance.is_null() {
            return;
        }
        unsafe { &*($instance as *const FMUInstance) }
    }};
}

// macro_rules! get_instance_mut {
//     ($instance:expr) => {{
//         if $instance.is_null() {
//             return;
//         }
//         unsafe { &mut *($instance as *mut FMUInstance) }
//     }};
// }

// macro_rules! get_fmu {
//     ($instance:expr) => {{
//         match $instance.fmu.as_ref() {
//         Some(fmu) => fmu,
//         None => {
//             let mut guard = $instance.errorMessages.lock().unwrap();
//             if !guard.is_empty() {   
//                 guard.push("FMU is not instantiated.".to_string());
//             }
//             return
//         },
//         }
//     }};
// }

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

pub mod common;
pub mod fmi2;
pub mod fmi3;