#![allow(non_camel_case_types, non_snake_case)]

use std::ffi::CStr;
use std::os::raw::c_char;

unsafe extern "C" {
    fn xmlInitParser();

    fn validate_xml_document(
        document_buffer: *const u8,
        document_buffer_size: i32,
        schema_buffer: *const u8,
        schema_buffer_size: i32,
        messages: *mut *mut *const i8,
        external_entity_loader: xmlExternalEntityLoader,
    ) -> i32;

    fn free_messages(len: i32, messages: *mut *const i8);

    fn xmlNewStringInputStream(ctxt: xmlParserCtxtPtr, value: *const xmlChar) -> xmlParserInputPtr;

    fn xmlNoNetExternalEntityLoader(
        URL: *const c_char,
        ID: *const c_char,
        ctxt: xmlParserCtxtPtr,
    ) -> xmlParserInputPtr;
}

// Type alias matching libxml2's: typedef xmlParserInputPtr (*xmlExternalEntityLoader) (...)
type xmlExternalEntityLoader = Option<
    unsafe extern "C" fn(
        URL: *const c_char,
        ID: *const c_char,
        ctxt: xmlParserCtxtPtr,
    ) -> xmlParserInputPtr,
>;

#[repr(C)]
struct xmlParserCtxt {
    _unused: [u8; 0],
}

#[repr(C)]
struct xmlParserInput {
    _unused: [u8; 0],
}

type xmlChar = u8;
type xmlParserCtxtPtr = *mut xmlParserCtxt;
type xmlParserInputPtr = *mut xmlParserInput;

const FMI2_ANNOTATION_XSD: &str = concat!(include_str!("fmi2/fmi2Annotation.xsd"), "\0");
const FMI2_ATTRIBUTE_GROUPS_XSD: &str = concat!(include_str!("fmi2/fmi2AttributeGroups.xsd"), "\0");
const FMI2_MODEL_DESCRIPTION_XSD: &str =
    concat!(include_str!("fmi2/fmi2ModelDescription.xsd"), "\0");
const FMI2_SCALAR_VARIABLE_XSD: &str = concat!(include_str!("fmi2/fmi2ScalarVariable.xsd"), "\0");
const FMI2_TYPE_XSD: &str = concat!(include_str!("fmi2/fmi2Type.xsd"), "\0");
const FMI2_UNIT_XSD: &str = concat!(include_str!("fmi2/fmi2Unit.xsd"), "\0");
const FMI2_VARIABLE_DEPENDENCY_XSD: &str =
    concat!(include_str!("fmi2/fmi2VariableDependency.xsd"), "\0");

const FMI3_ANNOTATION_XSD: &str = concat!(include_str!("fmi3/fmi3Annotation.xsd"), "\0");
const FMI3_ATTRIBUTE_GROUPS_XSD: &str = concat!(include_str!("fmi3/fmi3AttributeGroups.xsd"), "\0");
const FMI3_BUILD_DESCRIPTION_XSD: &str =
    concat!(include_str!("fmi3/fmi3BuildDescription.xsd"), "\0");
const FMI3_INTERFACE_TYPE_XSD: &str = concat!(include_str!("fmi3/fmi3InterfaceType.xsd"), "\0");
const FMI3_LAYERED_STANDARD_MANIFEST_XSD: &str =
    concat!(include_str!("fmi3/fmi3LayeredStandardManifest.xsd"), "\0");
const FMI3_MODEL_DESCRIPTION_XSD: &str =
    concat!(include_str!("fmi3/fmi3ModelDescription.xsd"), "\0");
const FMI3_TERMINAL_XSD: &str = concat!(include_str!("fmi3/fmi3Terminal.xsd"), "\0");
const FMI3_TERMINALS_AND_ICONS_XSD: &str =
    concat!(include_str!("fmi3/fmi3TerminalsAndIcons.xsd"), "\0");
const FMI3_TYPE_XSD: &str = concat!(include_str!("fmi3/fmi3Type.xsd"), "\0");
const FMI3_UNIT_XSD: &str = concat!(include_str!("fmi3/fmi3Unit.xsd"), "\0");
const FMI3_VARIABLE_XSD: &str = concat!(include_str!("fmi3/fmi3Variable.xsd"), "\0");
const FMI3_VARIABLE_DEPENDENCY_XSD: &str =
    concat!(include_str!("fmi3/fmi3VariableDependency.xsd"), "\0");

const FMI_LS_DAE_MANIFEST_XSD: &str =
    concat!(include_str!("dae/fmi3LayeredStandardDaeManifest.xsd"), "\0");

#[unsafe(no_mangle)]
unsafe extern "C" fn custom_entity_loader(
    url: *const c_char,
    id: *const c_char,
    ctxt: xmlParserCtxtPtr,
) -> xmlParserInputPtr {
    unsafe {
        if url.is_null() {
            return xmlNoNetExternalEntityLoader(url, id, ctxt);
        }

        let url_cstr = CStr::from_ptr(url);

        let url_str = match url_cstr.to_str() {
            Ok(s) => s,
            Err(_) => return xmlNoNetExternalEntityLoader(url, id, ctxt),
        };

        match url_str {
            // FMI 2.0
            "fmi2Annotation.xsd" => xmlNewStringInputStream(ctxt, FMI2_ANNOTATION_XSD.as_ptr()),
            "fmi2AttributeGroups.xsd" => {
                xmlNewStringInputStream(ctxt, FMI2_ATTRIBUTE_GROUPS_XSD.as_ptr())
            }
            "fmi2ModelDescription.xsd" => {
                xmlNewStringInputStream(ctxt, FMI2_MODEL_DESCRIPTION_XSD.as_ptr())
            }
            "fmi2ScalarVariable.xsd" => {
                xmlNewStringInputStream(ctxt, FMI2_SCALAR_VARIABLE_XSD.as_ptr())
            }
            "fmi2Type.xsd" => xmlNewStringInputStream(ctxt, FMI2_TYPE_XSD.as_ptr()),
            "fmi2Unit.xsd" => xmlNewStringInputStream(ctxt, FMI2_UNIT_XSD.as_ptr()),
            "fmi2VariableDependency.xsd" => {
                xmlNewStringInputStream(ctxt, FMI2_VARIABLE_DEPENDENCY_XSD.as_ptr())
            }
            // FMI 3.0
            "fmi3Annotation.xsd" => xmlNewStringInputStream(ctxt, FMI3_ANNOTATION_XSD.as_ptr()),
            "fmi3AttributeGroups.xsd" => {
                xmlNewStringInputStream(ctxt, FMI3_ATTRIBUTE_GROUPS_XSD.as_ptr())
            }
            "fmi3BuildDescription.xsd" => {
                xmlNewStringInputStream(ctxt, FMI3_BUILD_DESCRIPTION_XSD.as_ptr())
            }
            "fmi3InterfaceType.xsd" => {
                xmlNewStringInputStream(ctxt, FMI3_INTERFACE_TYPE_XSD.as_ptr())
            }
            "fmi3LayeredStandardManifest.xsd" => {
                xmlNewStringInputStream(ctxt, FMI3_LAYERED_STANDARD_MANIFEST_XSD.as_ptr())
            }
            "fmi3ModelDescription.xsd" => {
                xmlNewStringInputStream(ctxt, FMI3_MODEL_DESCRIPTION_XSD.as_ptr())
            }
            "fmi3Terminal.xsd" => xmlNewStringInputStream(ctxt, FMI3_TERMINAL_XSD.as_ptr()),
            "fmi3TerminalsAndIcons.xsd" => {
                xmlNewStringInputStream(ctxt, FMI3_TERMINALS_AND_ICONS_XSD.as_ptr())
            }
            "fmi3Type.xsd" => xmlNewStringInputStream(ctxt, FMI3_TYPE_XSD.as_ptr()),
            "fmi3Unit.xsd" => xmlNewStringInputStream(ctxt, FMI3_UNIT_XSD.as_ptr()),
            "fmi3Variable.xsd" => xmlNewStringInputStream(ctxt, FMI3_VARIABLE_XSD.as_ptr()),
            "fmi3VariableDependency.xsd" => {
                xmlNewStringInputStream(ctxt, FMI3_VARIABLE_DEPENDENCY_XSD.as_ptr())
            }
            _ => xmlNoNetExternalEntityLoader(url, id, ctxt),
        }
    }
}

/// Call this once to ensure the linker pulls in libxml2 symbols
/// and the library is initialized.
fn init_libxml() {
    unsafe {
        xmlInitParser();
    }
}

pub fn validate_fmi2_model_description(document: &[u8]) -> Vec<String> {
    let schema_buffer = FMI2_MODEL_DESCRIPTION_XSD.as_bytes();
    validate_xml_document_against_schema(document, schema_buffer, Some(custom_entity_loader))
}

pub fn validate_fmi3_model_description(document: &[u8]) -> Vec<String> {
    let schema_buffer = FMI3_MODEL_DESCRIPTION_XSD.as_bytes();
    validate_xml_document_against_schema(document, schema_buffer, Some(custom_entity_loader))
}

pub fn validate_build_description(document: &[u8]) -> Vec<String> {
    let schema_buffer = FMI3_BUILD_DESCRIPTION_XSD.as_bytes();
    validate_xml_document_against_schema(document, schema_buffer, Some(custom_entity_loader))
}

pub fn validate_dae_manifest(document: &[u8]) -> Vec<String> {
    let schema_buffer = FMI_LS_DAE_MANIFEST_XSD.as_bytes();
    validate_xml_document_against_schema(document, schema_buffer, Some(custom_entity_loader))
}

fn validate_xml_document_against_schema(
    document: &[u8],
    schema: &[u8],
    external_entity_loader: xmlExternalEntityLoader,
) -> Vec<String> {
    init_libxml();

    let mut messages: *mut *const i8 = std::ptr::null_mut();

    let n_messages = unsafe {
        validate_xml_document(
            document.as_ptr(),
            document.len() as i32,
            schema.as_ptr(),
            schema.len().saturating_sub(1) as i32,
            &mut messages,
            external_entity_loader,
        )
    };

    if n_messages > 0 && !messages.is_null() {
        let messages_slice = unsafe { std::slice::from_raw_parts(messages, n_messages as usize) };
        let messages_vec: Vec<String> = messages_slice
            .iter()
            .filter_map(|&msg_ptr| {
                if !msg_ptr.is_null() {
                    let msg_cstr = unsafe { std::ffi::CStr::from_ptr(msg_ptr) };
                    msg_cstr.to_str().ok().map(|s| s.trim_end().to_string())
                } else {
                    None
                }
            })
            .collect();
        unsafe { free_messages(n_messages, messages) };
        messages_vec
    } else {
        vec![]
    }
}
