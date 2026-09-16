use fmi_rs::model_description::validation::validate_structured_variable_name;

#[test]
fn test_structured_variable_name_validation() {
    let valid_names = vec![
        "_",
        "der(x)",
        "a.b.c",
        "v[1]",
        "v[1,2]",
        "'quoted.name'",
        "der('quoted name'[1])",
        "der(a[1].b[2,3].c)",
    ];

    for name in valid_names {
        assert_eq!(validate_structured_variable_name(name), Ok(()));
    }

    let invalid_names = vec![
        (
            "1x",
            "syntax error, unexpected UNSIGNED_INTEGER, expecting DER or NONDIGIT or Q_NAME",
        ),
        (
            "a..b",
            "syntax error, unexpected '.', expecting NONDIGIT or Q_NAME",
        ),
        (
            "a[1",
            "syntax error, unexpected end of file, expecting ',' or ']'",
        ),
        (
            "a[1,2",
            "syntax error, unexpected end of file, expecting ',' or ']'",
        ),
        (
            "a[1]b",
            "syntax error, unexpected NONDIGIT, expecting end of file",
        ),
        (
            "der(a[1].b[2].c",
            "syntax error, unexpected end of file, expecting ')' or ',' or '.'",
        ),
    ];

    for (name, expected_error) in invalid_names {
        assert_eq!(
            validate_structured_variable_name(name),
            Err(expected_error.to_string())
        );
    }
}
