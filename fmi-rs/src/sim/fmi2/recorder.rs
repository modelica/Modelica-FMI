use std::cell::RefCell;

use crate::{
    fmi2::FMU2,
    model_description::fmi2::VariableType,
    sim::{
        SimulationError,
        fmi2::{Trajectories, VariableValue, call},
    },
};

#[derive(Debug)]
pub struct Recorder {
    pub trajectories: RefCell<Trajectories>,
}

impl Recorder {
    pub fn new(trajectories: Trajectories) -> Self {
        Recorder {
            trajectories: RefCell::new(trajectories),
        }
    }

    pub fn sample<I>(&self, time: f64, fmu: &FMU2<I>) -> Result<(), SimulationError> {
        let mut trajectories = self.trajectories.borrow_mut();

        trajectories.time.push(time);

        let mut row = vec![];

        for variable in trajectories.variables() {
            let value_references = [variable.valueReference];

            let variable_value = match variable.variableType {
                VariableType::Real { .. } => {
                    let mut values = [0.0];
                    call(fmu.getReal(&value_references, &mut values))?;
                    VariableValue::Real(values[0])
                }
                VariableType::Integer { .. } | VariableType::Enumeration { .. } => {
                    let mut values = [0];
                    call(fmu.getInteger(&value_references, &mut values))?;
                    VariableValue::Integer(values[0])
                }
                VariableType::Boolean { .. } => {
                    let mut values = [0];
                    call(fmu.getBoolean(&value_references, &mut values))?;
                    VariableValue::Boolean(values[0])
                }
                VariableType::String { .. } => {
                    let mut values = [String::new()];
                    call(fmu.getString(&value_references, &mut values))?;
                    VariableValue::String(values[0].clone())
                }
            };

            row.push(variable_value);
        }

        trajectories.rows.push(row);

        Ok(())
    }

    pub fn into_trajectories(self) -> Trajectories {
        self.trajectories.into_inner()
    }
}
