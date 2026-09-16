use std::ffi::{CStr, CString, c_char};

unsafe extern "C" {
    unsafe fn validate_variable_name(name: *const c_char) -> *const c_char;
    unsafe fn free(ptr: *mut std::ffi::c_void);
}

pub fn validate_structured_variable_name(name: &str) -> Result<(), String> {
    let c_name = CString::new(name).map_err(|error| error.to_string())?;
    unsafe {
        let result = validate_variable_name(c_name.as_ptr());
        if result.is_null() {
            Ok(())
        } else {
            let s = CStr::from_ptr(result).to_string_lossy().into_owned();
            // Free the memory allocated by the C parser
            free(result as *mut std::ffi::c_void);
            Err(s)
        }
    }
}
