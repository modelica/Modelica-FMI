use std::sync::Arc;

use crate::fmi3::IntermediateUpdateHandler;
use crate::fmi3::log::DefaultLogger;
use crate::fmi3::types::{fmi3Boolean, fmi3Float64, fmi3ValueReference};
use crate::sim::fmi3::{
    SimulationSettings, call, read_initial_fmu_state, set_start_values, write_final_fmu_state,
};
use crate::sim::{
    SimulationError, next_communication_point, next_regular_point, validate_simulation_steps,
};
use crate::{
    fmi3::FMU3,
    sim::{
        fmi3::{input::StaticInput, recorder::Recorder},
        relative_eq, relative_le, relative_lt,
    },
};

struct IntermediateValueRecorder {
    input: Option<Arc<StaticInput>>,
    recorder: Arc<Recorder>,
}

impl IntermediateUpdateHandler for IntermediateValueRecorder {
    fn required_intermediate_variables(&self) -> Vec<fmi3ValueReference> {
        let mut value_references = vec![];

        if let Some(input) = &self.input {
            value_references.extend(input.trajectories.value_references());
        }

        value_references.extend(self.recorder.trajectories.borrow().value_references());

        value_references
    }

    fn intermediate_update(
        &self,
        fmu: &FMU3,
        intermediateUpdateTime: fmi3Float64,
        intermediateVariableSetRequested: fmi3Boolean,
        intermediateVariableGetAllowed: fmi3Boolean,
        intermediateStepFinished: fmi3Boolean,
        canReturnEarly: fmi3Boolean,
    ) -> (fmi3Boolean, fmi3Float64) {
        if intermediateVariableSetRequested
            && let Some(input) = &self.input
            && let Err(_) = input.set_continuous_inputs(intermediateUpdateTime, false, fmu)
        {
            return (canReturnEarly, intermediateUpdateTime);
        }

        if intermediateVariableGetAllowed
            && intermediateStepFinished
            && let Err(_) = self.recorder.sample(intermediateUpdateTime, fmu)
        {
            return (canReturnEarly, intermediateUpdateTime);
        }

        (false, 0.0)
    }
}

pub fn simulate(
    settings: &SimulationSettings,
    input: Option<Arc<StaticInput>>,
    recorder: Arc<Recorder>,
) -> Result<(), SimulationError> {
    let start_time = settings.start_time;
    let stop_time = settings.stop_time;
    let set_stop_time = settings.set_stop_time;
    let output_interval = settings.output_interval;
    let event_mode_used = settings.event_mode_used;
    let relative_tolerance = settings.tolerance;

    validate_simulation_steps(start_time, stop_time, output_interval, relative_tolerance)
        .map_err(SimulationError::Parameter)?;

    let mut time = start_time;

    let co_simulation = settings
        .model_description
        .coSimulation
        .as_ref()
        .ok_or(SimulationError::InterfaceType)?;

    let can_handle_variable_communication_step_size =
        co_simulation.canHandleVariableCommunicationStepSize;

    let logger = Arc::new(if let Some(log_file) = &settings.log_file {
        DefaultLogger::from_path(log_file).map_err(SimulationError::io(&log_file))?
    } else {
        DefaultLogger::default()
    });

    let intermediate_update_handler: Option<Box<dyn IntermediateUpdateHandler>> =
        if settings.intermediate_update {
            Some(Box::new(IntermediateValueRecorder {
                input: input.clone(),
                recorder: recorder.clone(),
            }))
        } else {
            None
        };

    let fmu = FMU3::instantiateCoSimulation(
        &settings.unzipdir,
        &co_simulation.modelIdentifier,
        &settings.model_description.modelName,
        &settings.model_description.instantiationToken,
        false,
        settings.logging_on,
        settings.event_mode_used,
        settings.early_return_allowed,
        logger,
        settings.log_fmi_calls,
        intermediate_update_handler,
    )?;

    if let Some(path) = &settings.initial_fmu_state_file {
        read_initial_fmu_state(&fmu, path)?;
        set_start_values(
            &settings.start_values,
            settings.model_description.as_ref(),
            &fmu,
        )?;
    } else {
        set_start_values(
            &settings.start_values,
            settings.model_description.as_ref(),
            &fmu,
        )?;

        call(fmu.enterInitializationMode(
            if settings.set_tolerance {
                Some(settings.tolerance)
            } else {
                None
            },
            start_time,
            if set_stop_time { Some(stop_time) } else { None },
        ))?;

        if let Some(input) = &input {
            input.set_discrete_inputs(time, &fmu)?;
            input.set_continuous_inputs(time, true, &fmu)?;
        }

        call(fmu.exitInitializationMode())?;

        if event_mode_used {
            loop {
                let mut discreteStatesNeedUpdate = false;
                let mut terminateSimulation = false;
                let mut nominalsOfContinuousStatesChanged = false;
                let mut valuesOfContinuousStatesChanged = false;
                let mut nextEventTime = None;

                call(fmu.updateDiscreteStates(
                    &mut discreteStatesNeedUpdate,
                    &mut terminateSimulation,
                    &mut nominalsOfContinuousStatesChanged,
                    &mut valuesOfContinuousStatesChanged,
                    &mut nextEventTime,
                ))?;

                if let Some(next_event_time) = nextEventTime
                    && relative_le(next_event_time, time, relative_tolerance)
                {
                    return Err(SimulationError::NextEventTime {
                        time,
                        next_event_time,
                    });
                }

                if terminateSimulation {
                    call(fmu.terminate())?;
                    return Ok(());
                }

                if !discreteStatesNeedUpdate {
                    break;
                }
            }

            call(fmu.enterStepMode())?;
        }
    }

    recorder.sample(time, &fmu)?;

    let mut n_steps = 0;

    let mut input_applied = false;

    while relative_lt(time, stop_time, relative_tolerance) {
        let next_regular_point = next_regular_point(
            settings.log_time_scale,
            start_time,
            output_interval,
            n_steps,
        )?;

        let next_input_event_time = input.as_ref().and_then(|i| i.next_event_time(time));

        let next_communication_point = if can_handle_variable_communication_step_size {
            next_communication_point(
                next_regular_point,
                next_input_event_time,
                None,
                stop_time,
                relative_tolerance,
            )
        } else {
            next_regular_point
        };

        if !input_applied && let Some(input) = &input {
            input.set_discrete_inputs(time, &fmu)?;
            input.set_continuous_inputs(time, !event_mode_used, &fmu)?;
        }

        let communication_step_size = next_communication_point - time;
        let mut event_handling_needed = false;
        let mut terminate_simulation = false;
        let mut early_return = false;
        let mut last_successful_time = 0.0;

        call(fmu.doStep(
            time,
            communication_step_size,
            true,
            &mut event_handling_needed,
            &mut terminate_simulation,
            &mut early_return,
            &mut last_successful_time,
        ))?;

        if early_return && !settings.early_return_allowed {
            return Err(SimulationError::Parameter(
                "The FMU returned early from fmi3DoStep() but early return is not allowed"
                    .to_owned(),
            ));
        }

        time = if early_return && last_successful_time < next_communication_point {
            last_successful_time
        } else {
            next_communication_point
        };

        if relative_eq(time, next_regular_point, relative_tolerance) {
            n_steps = n_steps.saturating_add(1);
        }

        recorder.sample(time, &fmu)?;

        if terminate_simulation {
            call(fmu.terminate())?;
            return Ok(());
        }

        let input_event = if let Some(next_input_event_time) = next_input_event_time {
            relative_eq(
                next_communication_point,
                next_input_event_time,
                relative_tolerance,
            )
        } else {
            false
        };

        input_applied = if event_mode_used && (input_event || event_handling_needed) {
            call(fmu.enterEventMode())?;

            if input_event && let Some(input) = &input {
                input.set_discrete_inputs(time, &fmu)?;
                input.set_continuous_inputs(time, true, &fmu)?;
            }

            loop {
                let mut discreteStatesNeedUpdate = false;
                let mut terminateSimulation = false;
                let mut nominalsOfContinuousStatesChanged = false;
                let mut valuesOfContinuousStatesChanged = false;
                let mut nextEventTime = None;

                call(fmu.updateDiscreteStates(
                    &mut discreteStatesNeedUpdate,
                    &mut terminateSimulation,
                    &mut nominalsOfContinuousStatesChanged,
                    &mut valuesOfContinuousStatesChanged,
                    &mut nextEventTime,
                ))?;

                if let Some(next_event_time) = nextEventTime
                    && relative_le(next_event_time, time, relative_tolerance)
                {
                    return Err(SimulationError::NextEventTime {
                        time,
                        next_event_time,
                    });
                }

                if terminateSimulation {
                    call(fmu.terminate())?;
                    return Ok(());
                }

                if !discreteStatesNeedUpdate {
                    break;
                }
            }

            call(fmu.enterStepMode())?;

            recorder.sample(time, &fmu)?;

            true
        } else {
            false
        };
    }

    if let Some(path) = &settings.final_fmu_state_file {
        write_final_fmu_state(&fmu, path)?;
    }

    call(fmu.terminate())?;

    Ok(())
}
