use crate::sim::{SimulationError, relative_eq};

pub trait Ode {
    fn nx(&self) -> usize;
    fn nz(&self) -> usize;
    fn init(&self, x: &mut [f64], nominals: &mut [f64]) -> Result<(), SimulationError>;
    fn f(&self, time: f64, x: &[f64], der_x: &mut [f64]) -> Result<(), SimulationError>;
    fn g(&self, time: f64, x: &[f64], z: &mut [f64]) -> Result<(), SimulationError>;
    fn supports_jacobian(&self) -> bool;
    fn jacobian(&self, time: f64, x: &[f64], J: &mut [f64]) -> Result<(), SimulationError>;
}

pub trait Dae {
    fn neq(&self) -> usize;
    fn nx(&self) -> usize;
    fn nz(&self) -> usize;
    fn init(
        &self,
        knowns: &mut [f64],
        nominals: &mut [f64],
        unknowns: &mut [f64],
    ) -> Result<(), SimulationError>;
    fn residuals(
        &self,
        time: f64,
        knowns: &[f64],
        unknowns: &[f64],
        residuals: &mut [f64],
    ) -> Result<(), SimulationError>;
    fn root(&self, time: f64, knowns: &[f64], z: &mut [f64]) -> Result<(), SimulationError>;
    fn jacobian(
        &self,
        time: f64,
        knowns: &[f64],
        alpha: f64,
        J: &mut [f64],
    ) -> Result<(), SimulationError>;
}

pub struct DummyDae;

impl Dae for DummyDae {
    fn neq(&self) -> usize {
        0
    }

    fn nx(&self) -> usize {
        0
    }

    fn nz(&self) -> usize {
        0
    }

    fn init(
        &self,
        _knowns: &mut [f64],
        _nominals: &mut [f64],
        _unknowns: &mut [f64],
    ) -> Result<(), SimulationError> {
        Err(SimulationError::Parameter("Not implemented".to_owned()))
    }

    fn residuals(
        &self,
        _time: f64,
        _knowns: &[f64],
        _unknowns: &[f64],
        _residuals: &mut [f64],
    ) -> Result<(), SimulationError> {
        Err(SimulationError::Parameter("Not implemented".to_owned()))
    }

    fn root(&self, _time: f64, _knowns: &[f64], _z: &mut [f64]) -> Result<(), SimulationError> {
        Err(SimulationError::Parameter("Not implemented".to_owned()))
    }

    fn jacobian(
        &self,
        _time: f64,
        _knowns: &[f64],
        _alpha: f64,
        _J: &mut [f64],
    ) -> Result<(), SimulationError> {
        Err(SimulationError::Parameter("Not implemented".to_owned()))
    }
}

pub trait Solver {
    fn reset(&mut self, time: f64) -> Result<(), SimulationError>;
    fn step(&mut self, next_time: f64) -> Result<(f64, &[f64], bool), SimulationError>;
}

pub trait SolverFactory {
    fn create<'a, O: Ode + 'a, D: Dae + 'a>(
        &self,
        start_time: f64,
        relative_tolerance: f64,
        ode: O,
        dae: Option<D>,
    ) -> Result<Box<dyn Solver + 'a>, SimulationError>;
}

pub struct ForwardEuler<T: Ode> {
    start_time: f64,
    fixed_step_size: f64,
    relative_tolerance: f64,
    n_steps: usize,
    x: Vec<f64>,
    nominals: Vec<f64>,
    der_x: Vec<f64>,
    z: Vec<f64>,
    pre_z: Vec<f64>,
    ode: T,
}

pub struct ForwardEulerFactory {
    pub fixed_step_size: f64,
}

impl SolverFactory for ForwardEulerFactory {
    fn create<'a, O: Ode + 'a, D: Dae + 'a>(
        &self,
        start_time: f64,
        relative_tolerance: f64,
        ode: O,
        _dae: Option<D>,
    ) -> Result<Box<dyn Solver + 'a>, SimulationError> {
        let nx = ode.nx();
        let nz = ode.nz();

        let mut x: Vec<f64> = vec![0.0; nx];
        let mut nominals: Vec<f64> = vec![0.0; nx];
        let der_x = vec![0.0; nx];
        let z = vec![0.0; nz];
        let mut pre_z = vec![0.0; nz];

        ode.init(x.as_mut_slice(), nominals.as_mut_slice())?;
        ode.g(start_time, x.as_slice(), pre_z.as_mut_slice())?;

        Ok(Box::new({
            ForwardEuler {
                start_time,
                fixed_step_size: self.fixed_step_size,
                relative_tolerance,
                n_steps: 0,
                x,
                nominals,
                der_x,
                z,
                pre_z,
                ode,
            }
        }))
    }
}

impl<T: Ode> ForwardEuler<T> {
    fn do_fixed_step(&mut self) -> Result<(f64, bool), SimulationError> {
        let time = self.start_time + self.n_steps as f64 * self.fixed_step_size;

        self.ode.f(time, &self.x, &mut self.der_x)?;

        for (x, der_x) in self.x.iter_mut().zip(self.der_x.iter()) {
            *x += *der_x * self.fixed_step_size;
        }

        self.n_steps = self.n_steps.saturating_add(1);

        let time = self.start_time + self.n_steps as f64 * self.fixed_step_size;

        self.ode.g(time, &self.x, &mut self.z)?;

        let mut state_event = false;

        for (z, pre_z) in self.z.iter().zip(self.pre_z.iter_mut()) {
            if (*pre_z <= 0.0 && *z > 0.0) || (*pre_z > 0.0 && *z <= 0.0) {
                state_event = true;
            }
            *pre_z = *z;
        }

        Ok((time, state_event))
    }
}

impl<T: Ode> Solver for ForwardEuler<T> {
    fn reset(&mut self, time: f64) -> Result<(), SimulationError> {
        self.start_time = time;
        self.n_steps = 0;
        self.ode.init(&mut self.x, &mut self.nominals)?;
        self.der_x.fill(0.0);
        self.z.fill(0.0);
        Ok(())
    }

    fn step(&mut self, next_time: f64) -> Result<(f64, &[f64], bool), SimulationError> {
        let mut time = self.start_time + self.n_steps as f64 * self.fixed_step_size;

        if next_time - time < self.fixed_step_size
            && !relative_eq(
                next_time,
                time + self.fixed_step_size,
                self.relative_tolerance,
            )
        {
            return Err(SimulationError::Parameter(format!(
                "Next time ({next_time}) is too close to current time ({time})"
            )));
        }

        while time + self.fixed_step_size < next_time
            || relative_eq(
                time + self.fixed_step_size,
                next_time,
                self.relative_tolerance,
            )
        {
            let (time_reached, state_event) = self.do_fixed_step()?;

            if state_event {
                return Ok((time_reached, &self.x, true));
            }

            time = time_reached;
        }

        Ok((time, &self.x, false))
    }
}
