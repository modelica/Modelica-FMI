#![allow(non_snake_case)]

macro_rules! get_instance {
    ($instance:expr) => {{
        if $instance.is_null() {
            return;
        }
        unsafe { &mut *($instance as *mut FMUInstance) }
    }};
}

pub mod common;
pub mod fmi2;
pub mod fmi3;
