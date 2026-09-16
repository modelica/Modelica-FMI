#![allow(clippy::unwrap_used)]

mod build {
    #[cfg(feature = "schema")]
    pub mod schema;
    #[cfg(feature = "sundials")]
    pub mod sundials;
}

use std::env;

fn main() {
    println!("cargo:rerun-if-changed=src/c/src/logger_proxy.c");
    println!("cargo:rerun-if-changed=src/c/src/variable_name_validator.c");
    println!("cargo:rerun-if-changed=src/c/src/structured_variable_name.tab.c");
    println!("cargo:rerun-if-changed=src/c/src/structured_variable_name.yy.c");
    println!("cargo:rerun-if-changed=src/c/include/structured_variable_name.tab.h");

    let target = env::var("TARGET").unwrap();

    cc::Build::new()
        .include("src/c/include")
        .file("src/c/src/logger_proxy.c")
        .compile("logger_proxy");

    let mut builder = cc::Build::new();

    builder
        .define("YY_NO_UNISTD_H", None)
        .include("src/c/include")
        .file("src/c/src/variable_name_validator.c")
        .file("src/c/src/structured_variable_name.tab.c")
        .file("src/c/src/structured_variable_name.yy.c");

    if !target.contains("msvc") {
        builder.flag("-Wno-implicit-function-declaration");
        builder.flag("-Wno-unused-function");
    }

    builder.compile("variable_name_validator");

    #[cfg(feature = "schema")]
    build::schema::build_libxml2();

    #[cfg(feature = "sundials")]
    build::sundials::build_sundials();
}
