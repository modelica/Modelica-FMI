use fmi_rs::build_description::BuildDescription;
use rstest::*;

#[rstest]
fn test_build_fmi2() {
    let path = "tests/resources/buildDescription.xml";
    let build_description =
        BuildDescription::from_file(path).expect("Failed to parse build description");
    assert_eq!(build_description.fmiVersion, "3.0");
}
