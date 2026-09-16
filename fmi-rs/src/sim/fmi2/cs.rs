use std::sync::Arc;

use crate::{
    fmi2::{
        CS, FMU2,
        log::DefaultLogger,
        types::{fmi2Status, fmi2StatusKind},
    },
    sim::{
        SimulationError,
        fmi2::{
            SimulationSettings, call, input::StaticInput, read_initial_fmu_state,
            recorder::Recorder, set_start_values, write_final_fmu_state,
        },
        next_communication_point, next_regular_point, relative_eq, relative_lt,
        validate_simulation_steps,
    },
};

pub fn simulate(
    settings: &SimulationSettings,
    input: Option<Arc<StaticInput>>,
    recorder: Arc<Recorder>,
) -> Result<(), SimulationError> {
    let start_time = settings.start_time;
    let stop_time = settings.stop_time;
    let set_stop_time = settings.set_stop_time;
    let output_interval = settings.output_interval;
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

    let fmu = FMU2::<CS>::new(
        &settings.unzipdir,
        &co_simulation.modelIdentifier,
        &settings.model_description.modelName,
        &settings.model_description.guid,
        false,
        settings.logging_on,
        settings.log_fmi_calls,
        logger,
        !co_simulation.canNotUseMemoryManagementFunctions,
    )?;

    if let Some(path) = &settings.initial_fmu_state_file {
        read_initial_fmu_state(&fmu, path)?;
        set_start_values(&settings.start_values, &settings.model_description, &fmu)?;
    } else {
        set_start_values(&settings.start_values, &settings.model_description, &fmu)?;

        call(fmu.setupExperiment(
            if settings.set_tolerance {
                Some(settings.tolerance)
            } else {
                None
            },
            time,
            if set_stop_time { Some(stop_time) } else { None },
        ))?;

        call(fmu.enterInitializationMode())?;

        if let Some(input) = &input {
            input.set_discrete_inputs(time, &fmu)?;
            input.set_continuous_inputs(time, true, &fmu)?;
        }

        call(fmu.exitInitializationMode())?;
    }

    recorder.sample(time, &fmu)?;

    let mut n_steps = 0;

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

        let communication_step_size = next_communication_point - time;

        if let Some(input) = &input {
            input.set_discrete_inputs(time, &fmu)?;
            input.set_continuous_inputs(time, true, &fmu)?;
        }

        let do_step_status = fmu.doStep(time, communication_step_size, 0);

        let mut terminate_simulation = 0;

        if do_step_status == fmi2Status::Discard {
            call(fmu.getRealStatus(&fmi2StatusKind::LastSuccessfulTime, &mut time))?;
            call(fmu.getBooleanStatus(&fmi2StatusKind::Terminated, &mut terminate_simulation))?;
        } else {
            call(do_step_status)?;
            time = next_communication_point;
        }

        if relative_eq(time, next_communication_point, relative_tolerance) {
            n_steps = n_steps.saturating_add(1);
        }

        recorder.sample(time, &fmu)?;

        if terminate_simulation != 0 {
            break;
        }
    }

    if let Some(path) = &settings.final_fmu_state_file {
        write_final_fmu_state(&fmu, path)?;
    }

    call(fmu.terminate())?;

    Ok(())
}
