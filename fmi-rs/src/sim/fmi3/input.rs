use itertools::{Itertools, izip};

use crate::{
    fmi3::{FMU3, types::fmi3Status},
    model_description::fmi3::Variability,
    sim::{
        SimulationError, SimulationSliceExt,
        fmi3::{Trajectories, VariableValue, set_variable_value},
        relative_ge, relative_gt,
    },
};

fn call(status: fmi3Status) -> Result<fmi3Status, SimulationError> {
    if matches!(status, fmi3Status::Ok | fmi3Status::Warning) {
        Ok(status)
    } else {
        Err(SimulationError::FMICall)
    }
}

#[derive(Debug)]
pub struct StaticInput {
    pub trajectories: Trajectories,
    pub tolerance: f64,
}

impl StaticInput {
    pub fn new(trajectories: Trajectories, tolerance: f64) -> Self {
        StaticInput {
            trajectories,
            tolerance,
        }
    }

    pub fn next_event_time(&self, time: f64) -> Option<f64> {
        for ((t0, row0), (t1, row1)) in self
            .trajectories
            .time
            .iter()
            .copied()
            .zip(self.trajectories.rows.iter())
            .tuple_windows()
        {
            if time >= t1 {
                // TODO: use is_close()
                continue;
            }

            if t0 == t1 {
                return Some(t0); // discrete change of a continuous variable
            }

            for (variable_index, value0, value1) in
                izip!(&self.trajectories.variable_indices, row0, row1)
            {
                if let Some(variable) = &self
                    .trajectories
                    .model_description
                    .modelVariables
                    .get(*variable_index)
                    && variable.variability != Variability::Continuous
                    && value0 != value1
                {
                    return Some(t1);
                }
            }
        }

        None
    }

    pub fn set_discrete_inputs(&self, time: f64, fmu: &FMU3) -> Result<(), SimulationError> {
        if self.trajectories.time.is_empty() {
            return Ok(());
        }

        let mut index = 0;

        for (i, t) in self.trajectories.time.iter().enumerate() {
            if *t > time {
                break;
            }
            index = i;
        }

        let row = self.trajectories.rows.try_get(index)?;

        for (variable_index, value) in self.trajectories.variable_indices.iter().zip(row.iter()) {
            let variable = self
                .trajectories
                .model_description
                .modelVariables
                .try_get(*variable_index)?;
            if variable.variability != Variability::Continuous {
                call(set_variable_value(fmu, variable.valueReference, value))?;
            }
        }

        Ok(())
    }

    pub fn set_continuous_inputs(
        &self,
        time: f64,
        after_event: bool,
        fmu: &FMU3,
    ) -> Result<(), SimulationError> {
        if self.trajectories.time.is_empty() {
            return Ok(());
        }

        let mut row_index = 0;

        // find the index
        while row_index < self.trajectories.time.len().saturating_sub(2) {
            let next_time = *self
                .trajectories
                .time
                .try_get(row_index.saturating_add(1))?;

            if (!after_event && relative_ge(next_time, time, self.tolerance))
                || (after_event && relative_gt(next_time, time, self.tolerance))
            {
                break;
            }

            row_index = row_index.saturating_add(1);
        }

        let row0 = &self.trajectories.rows.try_get(row_index)?;
        let row1 = &self
            .trajectories
            .rows
            .try_get(row_index.saturating_add(1))?;

        let t0 = self.trajectories.time.try_get(row_index)?;
        let t1 = self
            .trajectories
            .time
            .try_get(row_index.saturating_add(1))?;
        let t = ((time - t0) / (t1 - t0)).clamp(0.0, 1.0);

        for (i, variable_index) in self.trajectories.variable_indices.iter().enumerate() {
            let variable = self
                .trajectories
                .model_description
                .modelVariables
                .try_get(*variable_index)?;

            if variable.variability != Variability::Continuous {
                continue;
            }

            let value0 = row0.try_get(i)?;
            let value1 = row1.try_get(i)?;

            match value0 {
                VariableValue::Float32(values0) => {
                    if let VariableValue::Float32(values1) = value1 {
                        let interpolated_values: Vec<f32> = values0
                            .iter()
                            .zip(values1.iter())
                            .map(|(x0, x1)| *x0 + t as f32 * (*x1 - *x0))
                            .collect();
                        call(fmu.setFloat32(&[variable.valueReference], &interpolated_values))?;
                    }
                }
                VariableValue::Float64(values0) => {
                    if let VariableValue::Float64(values1) = value1 {
                        let interpolated_values: Vec<f64> = values0
                            .iter()
                            .zip(values1.iter())
                            .map(|(x0, x1)| *x0 + t * (*x1 - *x0))
                            .collect();
                        call(fmu.setFloat64(&[variable.valueReference], &interpolated_values))?;
                    }
                }
                _ => {
                    return Err(SimulationError::Parameter(
                        "Illegal type for continuous input".to_owned(),
                    ));
                }
            }
        }

        Ok(())
    }
}
