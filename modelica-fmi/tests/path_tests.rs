#![allow(clippy::unwrap_used)]

use std::path::{Path, PathBuf};

fn test_library_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("tests/resources")
}

#[test]
fn computes_modelica_path_for_nested_library_packages() {
    let model_path = test_library_root().join("MyLibrary/MyPackage/Controller_FMU_2.mo");

    assert_eq!(
        modelica_fmi::modelica_path(model_path.as_path()).unwrap(),
        vec!["MyLibrary", "MyPackage"]
    );
}

#[test]
fn computes_modelica_path_for_standalone_model() {
    let model_path = test_library_root().join("Controller_FMU_2.mo");

    assert_eq!(
        modelica_fmi::modelica_path(model_path.as_path()).unwrap(),
        Vec::<&str>::new()
    );
}
