use std::{error::Error, fs};
use tera::{Context, Tera};

const MODELICA_TEMPLATE: &str = include_str!("../templates/ExternalFMU.mo.tera");

fn main() -> Result<(), Box<dyn Error>> {
    let mut context = Context::new();
    context.insert("model_name", "ExternalFMU");
    context.insert("description", "Generated from a Tera template");

    let modelica = Tera::one_off(MODELICA_TEMPLATE, &context, false)?;
    fs::write("ExternalFMU.mo", modelica)?;

    println!("Created ExternalFMU.mo");
    Ok(())
}
