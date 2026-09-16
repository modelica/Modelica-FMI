use fmi_rs::model_description::fmi3::ModelDescription;

#[test]
fn retrieves_initial_model_variable_dimensions() {
    let description = ModelDescription::from_string(
        r#"<fmiModelDescription fmiVersion="3.0" modelName="Test" instantiationToken="token">
            <ModelVariables>
                <Float64 name="array" valueReference="1">
                    <Dimension start="3"/>
                    <Dimension valueReference="2"/>
                </Float64>
                <UInt64 name="size" valueReference="2" start="4"/>
            </ModelVariables>
            <ModelStructure/>
        </fmiModelDescription>"#,
    )
    .unwrap();

    let variable = description.variable_by_name("array").unwrap();
    assert_eq!(description.initial_size(variable).unwrap(), vec![3, 4]);
}

#[test]
fn rejects_non_uint64_dimension_variables() {
    let description = ModelDescription::from_string(
        r#"<fmiModelDescription fmiVersion="3.0" modelName="Test" instantiationToken="token">
            <ModelVariables>
                <Float64 name="array" valueReference="1">
                    <Dimension valueReference="2"/>
                </Float64>
                <Int32 name="size" valueReference="2" start="4"/>
            </ModelVariables>
            <ModelStructure/>
        </fmiModelDescription>"#,
    )
    .unwrap();

    let variable = description.variable_by_name("array").unwrap();
    let error = description.initial_size(variable).unwrap_err();

    assert!(error.to_string().contains("is not UInt64"));
}
