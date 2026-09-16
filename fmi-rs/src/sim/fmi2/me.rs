use std::sync::Arc;

use crate::{
    fmi2::{
        FMU2, ME,
        log::DefaultLogger,
        types::{fmi2False, fmi2Real, fmi2Status, fmi2ValueReference},
    },
    sim::{
        SimulationError, SimulationSliceExt,
        fmi2::{
            SimulationSettings, call, input::StaticInput, read_initial_fmu_state,
            recorder::Recorder, set_start_values, write_final_fmu_state,
        },
        next_communication_point, next_regular_point, relative_eq, relative_ge, relative_le,
        solver::{DummyDae, Ode, SolverFactory},
        validate_simulation_steps,
    },
};

pub fn simulate<S: SolverFactory>(
    settings: &SimulationSettings,
    solver_factory: &S,
    input: Option<Arc<StaticInput>>,
    recorder: Arc<Recorder>,
) -> Result<(), SimulationError> {
    let start_time = settings.start_time;
    let stop_time = settings.stop_time;
    let set_stop_time = settings.set_stop_time;
    let output_interval = settings.output_interval;
    let relative_tolerance = settings.tolerance;

    validate_simulation_steps(start_time, stop_time, output_interval, relative_tolerance)
        .map_err(|e| SimulationError::Parameter(e.to_string()))?;

    let mut time = start_time;

    let model_exchange = settings
        .model_description
        .modelExchange
        .as_ref()
        .ok_or(SimulationError::InterfaceType)?;

    let needs_completed_integrator_step = !model_exchange.completedIntegratorStepNotNeeded;

    let logger = Arc::new(if let Some(log_file) = &settings.log_file {
        DefaultLogger::from_path(log_file).map_err(SimulationError::io(&log_file))?
    } else {
        DefaultLogger::default()
    });

    let fmu = FMU2::<ME>::new(
        &settings.unzipdir,
        &model_exchange.modelIdentifier,
        &settings.model_description.modelName,
        &settings.model_description.guid,
        false,
        settings.logging_on,
        settings.log_fmi_calls,
        logger,
        !model_exchange.canNotUseMemoryManagementFunctions,
    )?;

    let mut next_event_time: Option<fmi2Real> = None;

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

        loop {
            let mut newDiscreteStatesNeeded: bool = false;
            let mut terminateSimulation: bool = false;
            let mut _nominalsOfContinuousStatesChanged: bool = false;
            let mut _valuesOfContinuousStatesChanged: bool = false;

            call(fmu.newDiscreteStates(
                &mut newDiscreteStatesNeeded,
                &mut terminateSimulation,
                &mut _nominalsOfContinuousStatesChanged,
                &mut _valuesOfContinuousStatesChanged,
                &mut next_event_time,
            ))?;

            if let Some(next_event_time) = next_event_time
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

            if !newDiscreteStatesNeeded {
                break;
            }
        }

        call(fmu.enterContinuousTimeMode())?;
    }

    let ode = create_ode(settings, input.clone(), &fmu)?;

    let mut solver = solver_factory.create(time, settings.tolerance, ode, None::<DummyDae>)?;

    let mut n_steps = 0;

    loop {
        recorder.sample(time, &fmu)?;

        if relative_ge(time, stop_time, relative_tolerance) {
            break;
        }

        let next_regular_point = next_regular_point(
            settings.log_time_scale,
            start_time,
            output_interval,
            n_steps,
        )?;

        let next_input_event_time = input.as_ref().and_then(|i| i.next_event_time(time));

        let next_communication_point = next_communication_point(
            next_regular_point,
            next_input_event_time,
            next_event_time,
            stop_time,
            relative_tolerance,
        );

        let is_input_event = if let Some(input_event_time) = next_input_event_time {
            relative_eq(
                input_event_time,
                next_communication_point,
                relative_tolerance,
            )
        } else {
            false
        };

        let is_time_event = next_event_time
            .is_some_and(|t| relative_eq(t, next_communication_point, relative_tolerance));

        let (time_reached, x, is_state_event) = solver.step(next_communication_point)?;

        time = time_reached;

        call(fmu.setTime(time))?;

        if !x.is_empty() {
            call(fmu.setContinuousStates(x))?;
        }

        if is_input_event && let Some(input) = &input {
            input.set_continuous_inputs(time, false, &fmu)?;
        }

        if relative_eq(time, next_regular_point, relative_tolerance) {
            n_steps = n_steps.saturating_add(1);
        }

        let is_step_event = if needs_completed_integrator_step {
            let mut is_step_event = fmi2False;
            let mut terminate_simulation = fmi2False;

            call(fmu.completedIntegratorStep(
                fmi2False,
                &mut is_step_event,
                &mut terminate_simulation,
            ))?;

            if terminate_simulation != fmi2False {
                call(fmu.terminate())?;
                return Ok(());
            }

            is_step_event != fmi2False
        } else {
            false
        };

        if is_input_event || is_time_event || is_state_event || is_step_event {
            recorder.sample(time, &fmu)?;

            call(fmu.enterEventMode())?;

            if is_input_event && let Some(input) = &input {
                input.set_discrete_inputs(time, &fmu)?;
                input.set_continuous_inputs(time, true, &fmu)?;
            }

            loop {
                let mut newDiscreteStatesNeeded: bool = false;
                let mut terminateSimulation: bool = false;
                let mut _nominalsOfContinuousStatesChanged: bool = false;
                let mut _valuesOfContinuousStatesChanged: bool = false;

                call(fmu.newDiscreteStates(
                    &mut newDiscreteStatesNeeded,
                    &mut terminateSimulation,
                    &mut _nominalsOfContinuousStatesChanged,
                    &mut _valuesOfContinuousStatesChanged,
                    &mut next_event_time,
                ))?;

                if let Some(next_event_time) = next_event_time
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

                if !newDiscreteStatesNeeded {
                    break;
                }
            }

            call(fmu.enterContinuousTimeMode())?;

            solver.reset(time)?;
        }
    }

    if let Some(path) = &settings.final_fmu_state_file {
        write_final_fmu_state(&fmu, path)?;
    }

    call(fmu.terminate())?;

    Ok(())
}

fn create_ode<'a>(
    settings: &SimulationSettings,
    input: Option<Arc<StaticInput>>,
    fmu: &'a FMU2<ME>,
) -> Result<Ode2<'a>, SimulationError> {
    let mut known_vrs = vec![];
    let mut unknown_vrs = vec![];

    let supports_jacobian: bool = settings
        .model_description
        .modelExchange
        .as_ref()
        .ok_or(SimulationError::InterfaceType)?
        .providesDirectionalDerivative;

    for unknown in &settings.model_description.derivatives {
        let unknown_variable = settings
            .model_description
            .variable_by_index(unknown.index)?;
        unknown_vrs.push(unknown_variable.valueReference);
        let known_index = unknown_variable.variableType.derivative()?;
        let known_variable = settings.model_description.variable_by_index(known_index)?;
        known_vrs.push(known_variable.valueReference);
    }

    let ode = Ode2 {
        fmu,
        input,
        nx: settings.model_description.derivatives.len(),
        nz: settings.model_description.numberOfEventIndicators as usize,
        supports_jacobian,
        known_vrs,
        unknown_vrs,
    };

    Ok(ode)
}

pub struct Ode2<'a> {
    fmu: &'a FMU2<ME>,
    input: Option<Arc<StaticInput>>,
    nx: usize,
    nz: usize,
    supports_jacobian: bool,
    known_vrs: Vec<fmi2ValueReference>,
    unknown_vrs: Vec<fmi2ValueReference>,
}

macro_rules! expect_ok {
    ($result:expr) => {
        if $result != fmi2Status::Ok {
            return Err(SimulationError::FMICall);
        }
    };
}

impl<'a> Ode for Ode2<'a> {
    fn nx(&self) -> usize {
        self.nx
    }

    fn nz(&self) -> usize {
        self.nz
    }

    fn init(&self, x: &mut [f64], nominals: &mut [f64]) -> Result<(), SimulationError> {
        expect_ok!(self.fmu.getContinuousStates(x));
        expect_ok!(self.fmu.getNominalsOfContinuousStates(nominals));
        Ok(())
    }

    fn f(&self, time: f64, x: &[f64], der_x: &mut [f64]) -> Result<(), SimulationError> {
        if self.nx > 0 {
            expect_ok!(self.fmu.setTime(time));

            if let Some(input) = &self.input {
                input.set_continuous_inputs(time, true, self.fmu)?;
            }

            expect_ok!(self.fmu.setContinuousStates(x));
            expect_ok!(self.fmu.getDerivatives(der_x));
        }
        Ok(())
    }

    fn g(&self, time: f64, x: &[f64], z: &mut [f64]) -> Result<(), SimulationError> {
        if self.nz > 0 {
            expect_ok!(self.fmu.setTime(time));
            expect_ok!(self.fmu.setContinuousStates(x));
            expect_ok!(self.fmu.getEventIndicators(z));
        }
        Ok(())
    }

    fn supports_jacobian(&self) -> bool {
        self.supports_jacobian
    }

    fn jacobian(&self, time: f64, x: &[f64], J: &mut [f64]) -> Result<(), SimulationError> {
        expect_ok!(self.fmu.setTime(time));

        if let Some(input) = &self.input {
            input.set_continuous_inputs(time, true, self.fmu)?;
        }

        expect_ok!(self.fmu.setContinuousStates(x));

        let mut seed = vec![0.0; self.nx];

        for i in 0..self.nx {
            seed.set(i, 1.0)?;
            let start = i.saturating_mul(self.nx);
            let end = i.saturating_add(1).saturating_mul(self.nx);
            let column = J.try_get_mut(start..end)?;
            expect_ok!(self.fmu.getDirectionalDerivative(
                &self.unknown_vrs,
                &self.known_vrs,
                &seed,
                column
            ));
            seed.set(i, 0.0)?;
        }

        Ok(())
    }
}
