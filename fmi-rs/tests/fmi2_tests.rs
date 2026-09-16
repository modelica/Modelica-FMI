#![allow(non_camel_case_types, non_snake_case)]

use core::f64;
use fmi_rs::fmi2::log::DefaultLogger;
use fmi_rs::fmi2::*;
use fmi_rs::model_description::fmi2::{Causality, ModelDescription};
use fmi_rs::sim::fmi2::recorder::Recorder;
use fmi_rs::sim::fmi2::{SimulationSettings, Trajectories};
use fmi_rs::{fmi2::types::*, sim::fmi2::cs::simulate};
use std::sync::Arc;
use std::vec;
use std::{env, path::PathBuf};

use fmi_rs::test_fixtures::download_reference_fmus;
use fmi_rs::zip::extract_zip_archive;

macro_rules! assert_ok {
    ($status:expr) => {
        assert_eq!($status, fmi2Status::Ok);
    };
}

#[test]
fn test_read_model_description() {
    let unzipdir: PathBuf =
        PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("tests/resources/fmi2/Feedthrough");
    ModelDescription::from_path(&unzipdir.join("modelDescription.xml")).unwrap();
}

fn create_fmu() -> FMU2<CS> {
    let resources_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("tests/resources/");

    let reference_fmus_dir = resources_dir.join("Reference-FMUs");

    if !reference_fmus_dir.exists() {
        download_reference_fmus(&reference_fmus_dir).unwrap();
    }

    let unzipdir = resources_dir.join("fmi2/Feedthrough");

    if !unzipdir.exists() {
        let fmu_path = reference_fmus_dir.join("2.0/Feedthrough.fmu");
        extract_zip_archive(&fmu_path, &unzipdir).unwrap();
    }

    let fmu = FMU2::<CS>::new(
        &unzipdir,
        "Feedthrough",
        "instance1",
        "{37B954F1-CC86-4D8F-B97F-C7C36F6670D2}",
        false,
        true,
        true,
        Box::new(DefaultLogger::default()),
        true,
    )
    .unwrap();

    assert_ok!(fmu.setupExperiment(None, 0.0, Some(1.0)));
    assert_ok!(fmu.enterInitializationMode());
    assert_ok!(fmu.exitInitializationMode());

    fmu
}

#[test]
fn test_csv_input() {
    let resources_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("tests")
        .join("resources");

    let unzipdir = resources_dir.join("fmi2").join("Feedthrough");

    let model_description =
        Arc::new(ModelDescription::from_path(&unzipdir.join("modelDescription.xml")).unwrap());

    let settings = SimulationSettings {
        unzipdir: unzipdir,
        model_description: model_description.clone(),
        start_time: 0.0,
        stop_time: 1.0,
        logging_on: true,
        set_stop_time: true,
        output_interval: 0.1,
        log_time_scale: false,
        tolerance: 0.0,
        set_tolerance: false,
        start_values: vec![],
        log_fmi_calls: true,
        input_file: Some(resources_dir.join("fmi2").join("Feedthrough_in.csv")),
        early_return_allowed: false,
        event_mode_used: false,
        log_file: None,
        initial_fmu_state_file: None,
        final_fmu_state_file: None,
    };

    let output_variables = settings
        .model_description
        .modelVariables
        .iter()
        .enumerate()
        .filter(|(_i, v)| v.causality == Causality::Output)
        .map(|(i, _v)| i)
        .collect();

    let output = Trajectories::new(model_description, output_variables);

    let recorder = Arc::new(Recorder::new(output));

    simulate(&settings, None, recorder).unwrap();
}

#[test]
fn test_real_continuous() {
    let fmu = create_fmu();

    let input_vr = [7];
    let input_values = [123.456789];

    let output_vr = [8];
    let mut output_values = [0.0];

    assert_ok!(fmu.setReal(&input_vr, &input_values));
    assert_ok!(fmu.getReal(&output_vr, &mut output_values));
    assert_eq!(output_values, input_values);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_real_discrete() {
    let fmu = create_fmu();

    let input_vr = [9];
    let input_values = [42.5];

    let output_vr = [10];
    let mut output_values = [0.0];

    assert_ok!(fmu.setReal(&input_vr, &input_values));
    assert_ok!(fmu.getReal(&output_vr, &mut output_values));
    assert_eq!(output_values, input_values);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_integer() {
    let fmu = create_fmu();

    let input_vr = [19];
    let input_values = [-987654321];

    let output_vr = [20];
    let mut output_values = [0];

    assert_ok!(fmu.setInteger(&input_vr, &input_values));
    assert_ok!(fmu.getInteger(&output_vr, &mut output_values));
    assert_eq!(output_values, input_values);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_boolean() {
    let fmu = create_fmu();

    let input_vr = [27];
    let input_values = [fmi2True];

    let output_vr = [28];
    let mut output_values = [fmi2False];

    assert_ok!(fmu.setBoolean(&input_vr, &input_values));
    assert_ok!(fmu.getBoolean(&output_vr, &mut output_values));
    assert_eq!(output_values, input_values);

    // Test with false value
    let input_values_false = [fmi2False];
    let mut output_values_false = [fmi2True];

    assert_ok!(fmu.setBoolean(&input_vr, &input_values_false));
    assert_ok!(fmu.getBoolean(&output_vr, &mut output_values_false));
    assert_eq!(output_values_false, input_values_false);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_string() {
    let fmu = create_fmu();

    let input_vr = [29];
    let input_values = ["Hello, FMI2!"];

    let output_vr = [30];
    let mut output_values = [String::new()];

    assert_ok!(fmu.setString(&input_vr, &input_values));
    assert_ok!(fmu.getString(&output_vr, &mut output_values));

    assert_eq!(output_values[0], input_values[0]);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_enumeration() {
    let fmu = create_fmu();

    let input_vr = [33];
    let input_values = [2]; // Option 2

    let output_vr = [34];
    let mut output_values = [0];

    assert_ok!(fmu.setInteger(&input_vr, &input_values));
    assert_ok!(fmu.getInteger(&output_vr, &mut output_values));
    assert_eq!(output_values, input_values);

    // Test with Option 1
    let input_values_option1 = [1];
    let mut output_values_option1 = [0];

    assert_ok!(fmu.setInteger(&input_vr, &input_values_option1));
    assert_ok!(fmu.getInteger(&output_vr, &mut output_values_option1));
    assert_eq!(output_values_option1, input_values_option1);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_multiple_variables() {
    let fmu = create_fmu();

    // Test setting and getting multiple different variable types in one test

    // Real
    let real_input_vr = [7];
    let real_input_values = [123.456];
    let real_output_vr = [8];
    let mut real_output_values = [0.0];

    // Integer
    let int_input_vr = [19];
    let int_input_values = [42];
    let int_output_vr = [20];
    let mut int_output_values = [0];

    // Boolean
    let bool_input_vr = [27];
    let bool_input_values = [fmi2True];
    let bool_output_vr = [28];
    let mut bool_output_values = [fmi2False];

    // String
    let string_input_vr = [29];
    let string_input_values = ["Multi-test"];
    let string_output_vr = [30];
    let mut string_output_values = [String::new()];

    // Set all input values
    assert_ok!(fmu.setReal(&real_input_vr, &real_input_values));
    assert_ok!(fmu.setInteger(&int_input_vr, &int_input_values));
    assert_ok!(fmu.setBoolean(&bool_input_vr, &bool_input_values));
    assert_ok!(fmu.setString(&string_input_vr, &string_input_values));

    // Get all output values
    assert_ok!(fmu.getReal(&real_output_vr, &mut real_output_values));
    assert_ok!(fmu.getInteger(&int_output_vr, &mut int_output_values));
    assert_ok!(fmu.getBoolean(&bool_output_vr, &mut bool_output_values));
    assert_ok!(fmu.getString(&string_output_vr, &mut string_output_values));

    // Verify all values
    assert_eq!(real_output_values, real_input_values);
    assert_eq!(int_output_values, int_input_values);
    assert_eq!(bool_output_values, bool_input_values);
    assert_eq!(string_output_values[0], string_input_values[0]);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_edge_cases() {
    let fmu = create_fmu();

    // Test extreme values for different types

    // Real edge cases
    let real_input_vr = [7];
    let real_output_vr = [8];

    // Test positive infinity
    let pos_inf = [f64::INFINITY];
    let mut output = [0.0];
    assert_ok!(fmu.setReal(&real_input_vr, &pos_inf));
    assert_ok!(fmu.getReal(&real_output_vr, &mut output));
    assert!(output[0].is_infinite() && output[0].is_sign_positive());

    // Test negative infinity
    let neg_inf = [f64::NEG_INFINITY];
    assert_ok!(fmu.setReal(&real_input_vr, &neg_inf));
    assert_ok!(fmu.getReal(&real_output_vr, &mut output));
    assert!(output[0].is_infinite() && output[0].is_sign_negative());

    // Test NaN
    let nan = [f64::NAN];
    assert_ok!(fmu.setReal(&real_input_vr, &nan));
    assert_ok!(fmu.getReal(&real_output_vr, &mut output));
    assert!(output[0].is_nan());

    // Test minimum and maximum integer values
    let int_input_vr = [19];
    let int_output_vr = [20];

    let max_int = [i32::MAX];
    let mut int_output = [0];
    assert_ok!(fmu.setInteger(&int_input_vr, &max_int));
    assert_ok!(fmu.getInteger(&int_output_vr, &mut int_output));
    assert_eq!(int_output, max_int);

    let min_int = [i32::MIN];
    assert_ok!(fmu.setInteger(&int_input_vr, &min_int));
    assert_ok!(fmu.getInteger(&int_output_vr, &mut int_output));
    assert_eq!(int_output, min_int);

    // Test very small and very large real numbers
    let very_small = [f64::MIN_POSITIVE];
    assert_ok!(fmu.setReal(&real_input_vr, &very_small));
    assert_ok!(fmu.getReal(&real_output_vr, &mut output));
    assert_eq!(output, very_small);

    let very_large = [f64::MAX];
    assert_ok!(fmu.setReal(&real_input_vr, &very_large));
    assert_ok!(fmu.getReal(&real_output_vr, &mut output));
    assert_eq!(output, very_large);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_parameters() {
    let fmu = create_fmu();

    // Test fixed parameter (should be settable during initialization)
    let fixed_param_vr = [5];
    let fixed_param_values = [f64::consts::PI];
    let mut output_values = [0.0];

    // Set parameter value
    assert_ok!(fmu.setReal(&fixed_param_vr, &fixed_param_values));
    assert_ok!(fmu.getReal(&fixed_param_vr, &mut output_values));
    assert_eq!(output_values, fixed_param_values);

    // Test tunable parameter
    let tunable_param_vr = [6];
    let tunable_param_values = [f64::consts::E];
    let mut tunable_output_values = [0.0];

    assert_ok!(fmu.setReal(&tunable_param_vr, &tunable_param_values));
    assert_ok!(fmu.getReal(&tunable_param_vr, &mut tunable_output_values));
    assert_eq!(tunable_output_values, tunable_param_values);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_simulation_step() {
    let fmu = create_fmu();

    // Set some input values
    let real_input_vr = [7];
    let real_input_values = [1.0];
    let int_input_vr = [19];
    let int_input_values = [100];

    assert_ok!(fmu.setReal(&real_input_vr, &real_input_values));
    assert_ok!(fmu.setInteger(&int_input_vr, &int_input_values));

    // Perform a simulation step
    let current_time = 0.0;
    let step_size = 0.1;
    assert_ok!(fmu.doStep(current_time, step_size, fmi2True));

    // Check that outputs are still correct after the step
    let real_output_vr = [8];
    let mut real_output_values = [0.0];
    let int_output_vr = [20];
    let mut int_output_values = [0];

    assert_ok!(fmu.getReal(&real_output_vr, &mut real_output_values));
    assert_ok!(fmu.getInteger(&int_output_vr, &mut int_output_values));

    assert_eq!(real_output_values, real_input_values);
    assert_eq!(int_output_values, int_input_values);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_empty_string() {
    let fmu = create_fmu();

    let input_vr = [29];
    let input_values = [""];

    let output_vr = [30];
    let mut output_values = [String::new()];

    assert_ok!(fmu.setString(&input_vr, &input_values));
    assert_ok!(fmu.getString(&output_vr, &mut output_values));

    assert_eq!(output_values[0], input_values[0]);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_long_string() {
    let fmu = create_fmu();

    let input_vr = [29];
    // Use a string that's within the 128 byte limit
    let long_string = "This is a long string that tests FMI2 string handling with special symbols: !@#$%^&*()_+-=[]{}|;':\",./<>?";
    let input_values = [long_string];

    let output_vr = [30];
    let mut output_values = [String::new()];

    assert_ok!(fmu.setString(&input_vr, &input_values));
    assert_ok!(fmu.getString(&output_vr, &mut output_values));

    assert_eq!(output_values[0], input_values[0]);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_zero_values() {
    let fmu = create_fmu();

    // Test zero real value
    let real_input_vr = [7];
    let real_input_values = [0.0];
    let real_output_vr = [8];
    let mut real_output_values = [1.0]; // Non-zero initial value

    assert_ok!(fmu.setReal(&real_input_vr, &real_input_values));
    assert_ok!(fmu.getReal(&real_output_vr, &mut real_output_values));
    assert_eq!(real_output_values, real_input_values);

    // Test zero integer value
    let int_input_vr = [19];
    let int_input_values = [0];
    let int_output_vr = [20];
    let mut int_output_values = [42]; // Non-zero initial value

    assert_ok!(fmu.setInteger(&int_input_vr, &int_input_values));
    assert_ok!(fmu.getInteger(&int_output_vr, &mut int_output_values));
    assert_eq!(int_output_values, int_input_values);

    assert_ok!(fmu.terminate());
}

#[test]
fn test_string_length_limit() {
    let fmu = create_fmu();

    let input_vr = [29];
    // Create a string that exceeds the 128 byte limit
    let too_long_string = "A".repeat(200);
    let input_values = [too_long_string.as_str()];

    // This should return an error due to string length limit
    let result = fmu.setString(&input_vr, &input_values);
    assert_eq!(result, fmi2Status::Error);
}
