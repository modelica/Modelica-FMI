use itertools::{Itertools, izip};

use crate::{
    fmi2::{FMU2, types::fmi2Status},
    model_description::fmi2::Variability,
    sim::{
        SimulationError, SimulationSliceExt,
        fmi2::{Trajectories, VariableValue, set_variable_value},
        relative_ge, relative_gt,
    },
};

fn call(status: fmi2Status) -> Result<fmi2Status, SimulationError> {
    if matches!(status, fmi2Status::Ok | fmi2Status::Warning) {
        Ok(status)
    } else {
        Err(SimulationError::FMICall)
    }
}

#[derive(Debug)]
pub struct StaticInput {
    trajectories: Trajectories,
    relative_tolerance: f64,
}

impl StaticInput {
    pub fn new(trajectories: Trajectories, relative_tolerance: f64) -> Self {
        StaticInput {
            trajectories,
            relative_tolerance,
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

    pub fn set_discrete_inputs<I>(&self, time: f64, fmu: &FMU2<I>) -> Result<(), SimulationError> {
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

        for (variable, value) in self.trajectories.variables().zip(row.iter()) {
            if variable.variability != Variability::Continuous {
                set_variable_value(fmu, variable.valueReference, value)?;
            }
        }

        Ok(())
    }

    pub fn set_continuous_inputs<I>(
        &self,
        time: f64,
        after_event: bool,
        fmu: &FMU2<I>,
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

            if (!after_event && relative_ge(next_time, time, self.relative_tolerance))
                || (after_event && relative_gt(next_time, time, self.relative_tolerance))
            {
                break;
            }

            row_index = row_index.saturating_add(1);
        }

        let row0 = self.trajectories.rows.try_get(row_index)?;
        let row1 = self
            .trajectories
            .rows
            .try_get(row_index.saturating_add(1))?;

        let t0 = self.trajectories.time.try_get(row_index)?;
        let t1 = self
            .trajectories
            .time
            .try_get(row_index.saturating_add(1))?;
        let t = ((time - t0) / (t1 - t0)).clamp(0.0, 1.0);

        for (variable, value0, value1) in izip!(self.trajectories.variables(), row0, row1) {
            if variable.variability != Variability::Continuous {
                continue;
            }

            match value0 {
                VariableValue::Real(value0) => {
                    if let VariableValue::Real(value1) = value1 {
                        let interpolated_value = value0 + t * (value1 - value0);
                        call(fmu.setReal(&[variable.valueReference], &[interpolated_value]))?;
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
