use std::{cell::RefCell, vec};

use crate::{
    fmi3::FMU3,
    model_description::fmi3::{Dimension, VariableType},
    sim::{
        SimulationError,
        fmi3::{Trajectories, VariableValue},
    },
};

#[derive(Debug)]
pub struct Recorder {
    pub trajectories: RefCell<Trajectories>,
    pub sizes: RefCell<Vec<usize>>,
}

impl Recorder {
    pub fn new(trajectories: Trajectories) -> Self {
        Recorder {
            trajectories: RefCell::new(trajectories),
            sizes: RefCell::new(vec![]),
        }
    }

    pub fn update_sizes(&self, fmu: &FMU3) {
        let mut sizes = self.sizes.borrow_mut();

        sizes.clear();

        let simulation_result = self.trajectories.borrow();

        for variable in simulation_result.variables() {
            let mut size = 1usize;
            for dimension in variable.dimensions.iter() {
                size = size.saturating_mul(match dimension {
                    Dimension::Fixed { start: size } => *size,
                    Dimension::Variable { valueReference } => {
                        let mut values = [0u64];
                        // TODO: handle status
                        fmu.getUInt64(&[*valueReference], &mut values);
                        values[0] as usize
                    }
                });
            }
            sizes.push(size);
        }
    }

    pub fn sample(&self, time: f64, fmu: &FMU3) -> Result<(), SimulationError> {
        if self.sizes.borrow().is_empty() {
            self.update_sizes(fmu);
        }

        // TODO: handle FMI status
        let mut trajectories = self.trajectories.borrow_mut();

        trajectories.time.push(time);

        let mut row = vec![];

        for (i, variable) in trajectories.variables().enumerate() {
            let size = self.sizes.borrow().get(i).copied().ok_or_else(|| {
                SimulationError::Parameter(format!("Missing size for recorded variable {i}"))
            })?;

            let value_references = [variable.valueReference];

            let variable_value = match variable.variableType {
                VariableType::Float32 { .. } => {
                    let mut values = vec![0f32; size];
                    fmu.getFloat32(&value_references, &mut values);
                    VariableValue::Float32(values)
                }
                VariableType::Float64 { .. } => {
                    let mut values = vec![0f64; size];
                    fmu.getFloat64(&value_references, &mut values);
                    VariableValue::Float64(values)
                }
                VariableType::Int8 { .. } => {
                    let mut values = vec![0i8; size];
                    fmu.getInt8(&value_references, &mut values);
                    VariableValue::Int8(values)
                }
                VariableType::UInt8 { .. } => {
                    let mut values = vec![0u8; size];
                    fmu.getUInt8(&value_references, &mut values);
                    VariableValue::UInt8(values)
                }
                VariableType::Int16 { .. } => {
                    let mut values = vec![0i16; size];
                    fmu.getInt16(&value_references, &mut values);
                    VariableValue::Int16(values)
                }
                VariableType::UInt16 { .. } => {
                    let mut values = vec![0u16; size];
                    fmu.getUInt16(&value_references, &mut values);
                    VariableValue::UInt16(values)
                }
                VariableType::Int32 { .. } => {
                    let mut values = vec![0i32; size];
                    fmu.getInt32(&value_references, &mut values);
                    VariableValue::Int32(values)
                }
                VariableType::UInt32 { .. } => {
                    let mut values = vec![0u32; size];
                    fmu.getUInt32(&value_references, &mut values);
                    VariableValue::UInt32(values)
                }
                VariableType::Int64 { .. } | VariableType::Enumeration { .. } => {
                    let mut values = vec![0i64; size];
                    fmu.getInt64(&value_references, &mut values);
                    VariableValue::Int64(values)
                }
                VariableType::UInt64 { .. } => {
                    let mut values = vec![0u64; size];
                    fmu.getUInt64(&value_references, &mut values);
                    VariableValue::UInt64(values)
                }
                VariableType::Boolean { .. } => {
                    let mut values = vec![false; size];
                    fmu.getBoolean(&value_references, &mut values);
                    VariableValue::Boolean(values)
                }
                VariableType::String { .. } => {
                    let mut values = vec![String::new(); size];
                    fmu.getString(&value_references, &mut values);
                    VariableValue::String(values)
                }
                VariableType::Binary { .. } => {
                    let mut values = vec![vec![]; size];
                    fmu.getBinary(&value_references, &mut values);
                    VariableValue::Binary(values)
                }
                _ => continue,
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
