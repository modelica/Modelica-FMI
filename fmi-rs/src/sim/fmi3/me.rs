use std::sync::Arc;

use crate::dae::DaeManifest;
use crate::fmi3::log::DefaultLogger;
use crate::model_description::ModelDescriptionError;
use crate::sim::fmi3::{SimulationSettings, call, set_start_values};
use crate::sim::solver::{Dae, Ode, SolverFactory};
use crate::sim::{
    SimulationError, SimulationSliceExt, next_communication_point, next_regular_point,
};
use crate::{
    fmi3::{FMU3, types::*},
    sim::{
        fmi3::{input::StaticInput, recorder::Recorder},
        relative_eq, relative_ge, relative_le,
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
    let relative_tolerance = settings.tolerance;

    let mut time = start_time;

    let model_exchange = settings
        .model_description
        .modelExchange
        .as_ref()
        .ok_or(SimulationError::InterfaceType)?;

    let needs_completed_integrator_step = model_exchange.needsCompletedIntegratorStep;

    let logger = Arc::new(if let Some(log_file) = &settings.log_file {
        DefaultLogger::from_path(log_file).map_err(SimulationError::io(&log_file))?
    } else {
        DefaultLogger::default()
    });

    let fmu = FMU3::instantiateModelExchange(
        &settings.unzipdir,
        &model_exchange.modelIdentifier,
        &settings.model_description.modelName,
        &settings.model_description.instantiationToken,
        false,
        settings.logging_on,
        logger,
        settings.log_fmi_calls,
    )?;

    set_start_values(
        &settings.start_values,
        settings.model_description.as_ref(),
        &fmu,
    )?;

    let (dae, algebraic_variable_vrs) = if settings.enable_dae {
        let (dae, vrs) = create_dae(settings, input.clone(), fmu.clone())?;
        (Some(dae), vrs)
    } else {
        (None, vec![])
    };

    call(fmu.enterInitializationMode(
        if settings.set_tolerance {
            Some(settings.tolerance)
        } else {
            None
        },
        time,
        if set_stop_time { Some(stop_time) } else { None },
    ))?;

    if let Some(input) = input.as_deref() {
        input.set_discrete_inputs(time, &fmu)?;
        input.set_continuous_inputs(time, false, &fmu)?;
    }

    call(fmu.exitInitializationMode())?;

    let mut next_event_time = None;

    // initial event iteration
    loop {
        let mut discreteStatesNeedUpdate = false;
        let mut terminateSimulation = false;
        let mut nominalsOfContinuousStatesChanged = false;
        let mut valuesOfContinuousStatesChanged = false;

        call(fmu.updateDiscreteStates(
            &mut discreteStatesNeedUpdate,
            &mut terminateSimulation,
            &mut nominalsOfContinuousStatesChanged,
            &mut valuesOfContinuousStatesChanged,
            &mut next_event_time,
        ))?;

        if terminateSimulation {
            call(fmu.terminate())?;
            return Ok(());
        }

        if !discreteStatesNeedUpdate {
            break;
        }
    }

    call(fmu.enterContinuousTimeMode())?;

    let ode = create_ode(settings, input.clone(), fmu.clone())?;

    let mut solver = solver_factory.create(time, settings.tolerance, ode, dae)?;

    let mut n_steps = 0;

    loop {
        recorder.sample(time, &fmu)?;

        if relative_ge(time, stop_time, relative_tolerance) {
            break;
        }

        let next_regular_point = next_regular_point(
            settings.log_time_scale,
            start_time,
            settings.output_interval,
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

        let is_time_event = if let Some(next_event_time) = next_event_time
            && relative_eq(
                next_event_time,
                next_communication_point,
                relative_tolerance,
            ) {
            true
        } else {
            false
        };

        let (time_reached, knowns, is_state_event) = solver.step(next_communication_point)?;

        time = time_reached;

        call(fmu.setTime(time))?;

        if !knowns.is_empty() {
            let nx = knowns.len().saturating_sub(algebraic_variable_vrs.len());
            call(fmu.setContinuousStates(knowns.try_get(0..nx)?))?;
            if !algebraic_variable_vrs.is_empty() {
                call(fmu.setFloat64(&algebraic_variable_vrs, knowns.try_get(nx..)?))?;
            }
        }

        if is_input_event && let Some(input) = &input {
            input.set_continuous_inputs(time, false, &fmu)?;
        }

        if relative_eq(time, next_regular_point, relative_tolerance) {
            n_steps = n_steps.saturating_add(1);
        }

        let mut is_step_event = false;

        if needs_completed_integrator_step {
            let mut terminate_simulation = false;

            call(fmu.completedIntegratorStep(
                false,
                &mut is_step_event,
                &mut terminate_simulation,
            ))?;

            if terminate_simulation {
                call(fmu.terminate())?;
                return Ok(());
            }
        }

        if is_input_event || is_time_event || is_state_event || is_step_event {
            recorder.sample(time, &fmu)?;

            call(fmu.enterEventMode())?;

            if is_input_event && let Some(input) = &input {
                input.set_discrete_inputs(time, &fmu)?;
                input.set_continuous_inputs(time, true, &fmu)?;
            }

            loop {
                let mut discreteStatesNeedUpdate = false;
                let mut terminateSimulation = false;
                let mut nominalsOfContinuousStatesChanged = false;
                let mut valuesOfContinuousStatesChanged = false;

                call(fmu.updateDiscreteStates(
                    &mut discreteStatesNeedUpdate,
                    &mut terminateSimulation,
                    &mut nominalsOfContinuousStatesChanged,
                    &mut valuesOfContinuousStatesChanged,
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

                if !discreteStatesNeedUpdate {
                    break;
                }
            }

            call(fmu.enterContinuousTimeMode())?;

            solver.reset(time)?;
        }
    }

    call(fmu.terminate())?;

    Ok(())
}

pub struct Ode3 {
    fmu: Arc<FMU3>,
    input: Option<Arc<StaticInput>>,
    nx: usize,
    nz: usize,
    supports_jacobian: bool,
    known_vrs: Vec<fmi3ValueReference>,
    unknown_vrs: Vec<fmi3ValueReference>,
}

macro_rules! expect_ok {
    ($result:expr) => {
        if $result != fmi3Status::Ok {
            return Err(SimulationError::FMICall);
        }
    };
}

impl Ode for Ode3 {
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
        expect_ok!(self.fmu.setTime(time));

        if let Some(input) = &self.input {
            input.set_continuous_inputs(time, true, &self.fmu)?;
        }

        if self.nx > 0 {
            expect_ok!(self.fmu.setContinuousStates(x));
            expect_ok!(self.fmu.getContinuousStateDerivatives(der_x));
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
            input.set_continuous_inputs(time, true, &self.fmu)?;
        }

        expect_ok!(self.fmu.setContinuousStates(x));

        for i in 0..self.nx {
            let mut seed = vec![0.0; self.nx];
            seed.set(i, 1.0)?;
            let column = J.try_get_mut(
                i.saturating_mul(self.nx)..i.saturating_add(1).saturating_mul(self.nx),
            )?;
            expect_ok!(self.fmu.getDirectionalDerivative(
                &self.unknown_vrs,
                &self.known_vrs,
                &seed,
                column
            ));
        }

        Ok(())
    }
}

fn create_ode(
    settings: &SimulationSettings,
    input: Option<Arc<StaticInput>>,
    fmu: Arc<FMU3>,
) -> Result<Ode3, SimulationError> {
    let mut nx = 0;
    let mut nz = 0;

    call(fmu.getNumberOfContinuousStates(&mut nx))?;
    call(fmu.getNumberOfEventIndicators(&mut nz))?;

    let mut known_vrs = vec![];
    let mut unknown_vrs = vec![];

    let supports_jacobian: bool = settings
        .model_description
        .modelExchange
        .as_ref()
        .ok_or(SimulationError::InterfaceType)?
        .providesDirectionalDerivatives;

    for unknown in &settings.model_description.derivatives {
        unknown_vrs.push(unknown.valueReference);
        known_vrs.push(
            settings
                .model_description
                .variable_by_value_reference(unknown.valueReference)?
                .variableType
                .derivative()
                .ok_or(ModelDescriptionError::ValueReference(
                    unknown.valueReference,
                ))?,
        );
    }

    let ode = Ode3 {
        fmu,
        input,
        nx,
        nz,
        supports_jacobian,
        known_vrs,
        unknown_vrs,
    };

    Ok(ode)
}

pub struct Dae3 {
    fmu: Arc<FMU3>,
    input: Option<Arc<StaticInput>>,
    nx: usize,
    nz: usize,
    known_vrs: Vec<fmi3ValueReference>,
    unknown_vrs: Vec<fmi3ValueReference>,
    algebraic_variable_nominals: Vec<fmi3Float64>,
}

impl Dae3 {
    pub fn new(
        fmu: Arc<FMU3>,
        input: Option<Arc<StaticInput>>,
        known_vrs: Vec<fmi3ValueReference>,
        unknown_vrs: Vec<fmi3ValueReference>,
        algebraic_variable_nominals: Vec<fmi3Float64>,
    ) -> Result<Self, SimulationError> {
        let mut nx = 0;
        expect_ok!(fmu.getNumberOfContinuousStates(&mut nx));

        let mut nz = 0;
        expect_ok!(fmu.getNumberOfEventIndicators(&mut nz));

        Ok(Self {
            fmu,
            input,
            nx,
            nz,
            known_vrs,
            unknown_vrs,
            algebraic_variable_nominals,
        })
    }
}

impl Dae for Dae3 {
    fn neq(&self) -> usize {
        self.known_vrs.len()
    }

    fn nx(&self) -> usize {
        self.nx
    }

    fn nz(&self) -> usize {
        self.nz
    }

    fn init(
        &self,
        knowns: &mut [f64],
        nominals: &mut [f64],
        unknowns: &mut [f64],
    ) -> Result<(), SimulationError> {
        expect_ok!(self.fmu.getFloat64(&self.known_vrs, knowns));
        expect_ok!(self.fmu.getFloat64(&self.unknown_vrs, unknowns));
        let continuous_state_nominals = nominals.try_get_mut(0..self.nx)?;
        expect_ok!(
            self.fmu
                .getNominalsOfContinuousStates(continuous_state_nominals)
        );
        for (i, v) in self.algebraic_variable_nominals.iter().enumerate() {
            nominals.set(self.nx.saturating_add(i), *v)?;
        }
        Ok(())
    }

    fn residuals(
        &self,
        time: f64,
        knowns: &[f64],
        unknowns: &[f64],
        residuals: &mut [f64],
    ) -> Result<(), SimulationError> {
        expect_ok!(self.fmu.setTime(time));

        if let Some(input) = &self.input {
            input.set_continuous_inputs(time, true, &self.fmu)?;
        }

        expect_ok!(self.fmu.setFloat64(&self.known_vrs, knowns));

        expect_ok!(self.fmu.getFloat64(&self.unknown_vrs, residuals));

        for (residual, unknown) in residuals.iter_mut().zip(unknowns) {
            *residual -= *unknown;
        }

        Ok(())
    }

    fn root(&self, time: f64, knowns: &[f64], z: &mut [f64]) -> Result<(), SimulationError> {
        expect_ok!(self.fmu.setTime(time));
        if let Some(input) = &self.input {
            input.set_continuous_inputs(time, true, &self.fmu)?;
        }
        expect_ok!(self.fmu.setFloat64(&self.known_vrs, knowns));
        expect_ok!(self.fmu.getEventIndicators(z));
        Ok(())
    }

    fn jacobian(
        &self,
        time: f64,
        knowns: &[f64],
        alpha: f64,
        J: &mut [f64],
    ) -> Result<(), SimulationError> {
        expect_ok!(self.fmu.setTime(time));

        if let Some(input) = &self.input {
            input.set_continuous_inputs(time, true, &self.fmu)?;
        }

        expect_ok!(self.fmu.setFloat64(&self.known_vrs, knowns));

        let n = self.known_vrs.len();

        for i in 0..n {
            let mut seed = vec![0.0; n];
            seed.set(i, 1.0)?;

            let column =
                J.try_get_mut(i.saturating_mul(n)..i.saturating_add(1).saturating_mul(n))?;

            expect_ok!(self.fmu.getDirectionalDerivative(
                &self.unknown_vrs,
                &self.known_vrs,
                &seed,
                column
            ));

            if i < self.nx {
                let slot = column.try_get_mut(i)?;
                *slot -= alpha;
            }
        }

        Ok(())
    }
}

fn create_dae(
    settings: &SimulationSettings,
    input: Option<Arc<StaticInput>>,
    fmu: Arc<FMU3>,
) -> Result<(Dae3, Vec<u32>), SimulationError> {
    let dae_manifest_path = settings
        .unzipdir
        .join("extra")
        .join("org.fmi-standard.fmi-ls-dae")
        .join("fmi-ls-manifest.xml");
    let dae_manifest = DaeManifest::from_file(dae_manifest_path)?;

    let mut continuous_state_vrs = vec![];
    let mut continuous_state_derivative_vrs = vec![];
    let mut algebraic_variable_vrs = vec![];
    let mut algebraic_variable_nominals = vec![];

    for derivative in dae_manifest.modelStructure.continuousStateDerivatives {
        continuous_state_derivative_vrs.push(derivative.valueReference);

        let derivative_variable = settings
            .model_description
            .variable_by_value_reference(derivative.valueReference)?;

        let continuous_state_vr =
            derivative_variable
                .variableType
                .derivative()
                .ok_or_else(|| {
                    SimulationError::Parameter(format!(
                        "Variable '{}' is missing the derivative attribute",
                        derivative_variable.name
                    ))
                })?;

        continuous_state_vrs.push(continuous_state_vr);
    }

    for algebraic_variable in &dae_manifest.algebraicVariables.algebraicVariables {
        algebraic_variable_vrs.push(algebraic_variable.valueReference);
        let variable = settings
            .model_description
            .variable_by_value_reference(algebraic_variable.valueReference)?;
        algebraic_variable_nominals.push(variable.variableType.nominal().unwrap_or(1.0));
    }

    let residual_vrs: Vec<u32> = dae_manifest
        .modelStructure
        .residuals
        .iter()
        .map(|r| r.valueReference)
        .collect();

    let known_vrs: Vec<u32> = continuous_state_vrs
        .clone()
        .into_iter()
        .chain(algebraic_variable_vrs.clone())
        .collect();

    let unknown_vrs: Vec<u32> = continuous_state_derivative_vrs
        .clone()
        .into_iter()
        .chain(residual_vrs)
        .collect();

    call(fmu.enterConfigurationMode())?;
    call(fmu.setBoolean(&[dae_manifest.enableDaeParameter.valueReference], &[true]))?;
    call(fmu.exitConfigurationMode())?;

    let dae = Dae3::new(
        fmu,
        input,
        known_vrs,
        unknown_vrs,
        algebraic_variable_nominals,
    )?;

    Ok((dae, algebraic_variable_vrs))
}
