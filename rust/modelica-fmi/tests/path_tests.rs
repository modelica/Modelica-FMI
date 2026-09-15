use std::path::Path;

#[test]
fn computes_modelica_within_path_for_nested_library_packages() {
    let model_path = Path::new(r"E:\WS\Modelica-FMI\FMI\Examples\FMI2\Controller_FMU_2.mo");

    assert_eq!(
        modelica_fmi::modelica_within_path(model_path).unwrap(),
        "FMI.Examples.FMI2".to_owned()
    );
}

#[test]
fn computes_modelica_within_path_standalone_model() {
    let model_path = Path::new(r"C:\Users\tsr2\Documents\Dymola\Controller_FMU_2.mo");

    assert_eq!(
        modelica_fmi::modelica_within_path(model_path).unwrap(),
        String::new()
    );
}
