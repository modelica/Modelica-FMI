use crate::sim::solver::{Dae, Ode, Solver, SolverFactory};
use crate::sim::{
    GetContinuousStateDerivativesFn, GetContinuousStatesFn, GetDirectionalDerivativeFn,
    GetEventIndicatorsFn, GetNominalsOfContinuousStatesFn, SetContinuousInputsFn,
    SetContinuousStatesFn, SetTimeFn, SimulationError,
};
use crate::sundials::cvode::CV_BDF;
use crate::sundials::nvector_serial::{NV_DATA_S, NV_LENGTH_S};
use crate::sundials::{
    cvode::{
        CV_NORMAL, CV_ROOT_RETURN, CVode, CVodeCreate, CVodeFree, CVodeInit, CVodeReInit,
        CVodeRootInit, CVodeSVtolerances, CVodeSetUserData,
    },
    cvode_ls::{CVodeSetJacFn, CVodeSetLinearSolver},
    nvector_serial::N_VNew_Serial,
    sundials_context::{SUNContext_Create, SUNContext_Free},
    sundials_linearsolver::{SUNLinSolFree, SUNLinearSolver},
    sundials_matrix::{SUNMatDestroy, SUNMatrix},
    sundials_nvector::{N_VDestroy, N_Vector},
    sundials_types::{SUN_COMM_NULL, SUNContext, sunindextype, sunrealtype},
    sunlinsol_dense::SUNLinSol_Dense,
    sunmatrix_dense::{SM_COLUMN_D, SUNDenseMatrix},
};
use std::{ffi::c_void, slice::from_raw_parts_mut};

macro_rules! expect_ok {
    ($result:expr) => {
        if let Err(_) = $result {
            return -1; // Indicate failure to CVODE
        }
    };
}

macro_rules! expect_no_error {
    ($flag:expr, $message:expr) => {
        if $flag != 0 {
            return Err(SimulationError::Solver(format!(
                "{}: error code {}",
                $message, $flag
            )));
        }
    };
}

macro_rules! expect_not_null {
    ($ptr:expr, $message:expr) => {
        if $ptr.is_null() {
            return Err(SimulationError::Solver($message.into()));
        }
    };
}

pub struct CVodeSolver<O: Ode> {
    sunctx: SUNContext,
    x: N_Vector,
    rtol: f64,
    abstol: N_Vector,
    A: SUNMatrix,
    LS: SUNLinearSolver,
    cvode_mem: *mut c_void,
    ode: Box<O>,
}

pub struct CVodeSolverFactory;

impl SolverFactory for CVodeSolverFactory {
    fn create<'a, T: Ode + 'a, D: Dae + 'a>(
        &self,
        start_time: f64,
        relative_tolerance: f64,
        ode: T,
        _dae: Option<D>,
    ) -> Result<Box<dyn Solver + 'a>, SimulationError> {
        unsafe {
            let mut sunctx = std::ptr::null_mut();

            expect_no_error!(
                SUNContext_Create(SUN_COMM_NULL, &mut sunctx),
                "Failed to create SUNDIALS context"
            );

            let cvode_mem = CVodeCreate(CV_BDF, sunctx);
            expect_not_null!(cvode_mem, "Failed to create CVODE memory");

            let ode = Box::new(ode);

            let nx = ode.nx();
            let nz = ode.nz();

            // let user_data: *const Functions = &*functions;
            let user_data: *const T = &*ode;

            expect_no_error!(
                CVodeSetUserData(cvode_mem, user_data as *mut c_void),
                "Failed to set user data"
            );

            let x = N_VNew_Serial(nx.max(1) as sunindextype, sunctx);
            expect_not_null!(x, "Failed to create N_Vector");

            let abstol = N_VNew_Serial(NV_LENGTH_S(x), sunctx);
            expect_not_null!(abstol, "Failed to create N_Vector");

            let x_slice = (*x).as_mut();
            let abstol_slice = (*abstol).as_mut();

            if nx > 0 {
                ode.init(x_slice, abstol_slice)?;
                for value in abstol_slice.iter_mut() {
                    *value *= relative_tolerance;
                }
            } else {
                x_slice.fill(0.0); // Dummy state for discrete systems
                abstol_slice.fill(1.0);
            }

            expect_no_error!(
                CVodeInit(cvode_mem, f::<T>, start_time, x),
                "Failed to initialize CVODE"
            );

            expect_no_error!(
                CVodeSVtolerances(cvode_mem, relative_tolerance, abstol),
                "Failed to set tolerances"
            );

            let A = SUNDenseMatrix(NV_LENGTH_S(x), NV_LENGTH_S(x), sunctx);
            expect_not_null!(A, "Failed to create dense matrix");

            let LS = SUNLinSol_Dense(x, A, sunctx);
            expect_not_null!(LS, "Failed to create linear solver");

            expect_no_error!(
                CVodeSetLinearSolver(cvode_mem, LS, A),
                "Failed to set linear solver"
            );

            if nx > 0 && ode.supports_jacobian() {
                expect_no_error!(
                    CVodeSetJacFn(cvode_mem, jac::<T>),
                    "Failed to set Jacobian function"
                );
            }

            if nz > 0 {
                expect_no_error!(
                    CVodeRootInit(cvode_mem, nz as i32, g::<T>),
                    "Failed to initialize rootfinding"
                );
            }

            Ok(Box::new(CVodeSolver {
                sunctx,
                x,
                rtol: relative_tolerance,
                abstol,
                A,
                LS,
                cvode_mem,
                ode,
            }))
        }
    }
}

impl<T: Ode> Solver for CVodeSolver<T> {
    fn reset(&mut self, time: f64) -> Result<(), SimulationError> {
        unsafe {
            let x = (*self.x).as_mut();
            let abstol = (*self.abstol).as_mut();

            if self.ode.nx() > 0 {
                self.ode.init(x, abstol)?;
                for v in abstol.iter_mut() {
                    *v *= self.rtol;
                }
            } else {
                x.fill(0.0);
                abstol.fill(1.0);
            }

            expect_no_error!(
                CVodeReInit(self.cvode_mem, time, self.x),
                "CVodeReInit failed"
            );
        }

        Ok(())
    }

    fn step(&mut self, next_time: f64) -> Result<(f64, &[f64], bool), SimulationError> {
        unsafe {
            let mut tret = 0.0;

            let flag = CVode(self.cvode_mem, next_time, self.x, &mut tret, CV_NORMAL);

            if flag < 0 {
                return Err(SimulationError::Solver(format!("status {flag}")));
            }

            let x: &[f64] = if self.ode.nx() > 0 {
                (*self.x).as_mut()
            } else {
                &[]
            };

            Ok((tret, x, flag == CV_ROOT_RETURN))
        }
    }
}

impl<T: Ode> Drop for CVodeSolver<T> {
    fn drop(&mut self) {
        unsafe {
            N_VDestroy(self.x);
            N_VDestroy(self.abstol);
            CVodeFree(&mut self.cvode_mem);
            SUNLinSolFree(self.LS);
            SUNMatDestroy(self.A);
            SUNContext_Free(&mut self.sunctx);
        }
    }
}

// Right-hand-side function
extern "C" fn f<T: Ode>(
    t: sunrealtype,
    y: N_Vector,
    ydot: N_Vector,
    user_data: *mut c_void,
) -> i32 {
    if user_data.is_null() {
        return -1;
    }

    unsafe {
        let ode: &mut T = &mut *(user_data as *mut T);
        let der_x = (*ydot).as_mut();

        if ode.nx() > 0 {
            ode.f(t, (*y).as_mut(), der_x).map_or(-1, |_| 0)
        } else {
            der_x.fill(0.0); // Dummy derivative for discrete systems
            0
        }
    }
}

// Root function
extern "C" fn g<T: Ode>(
    t: sunrealtype,
    y: N_Vector,
    gout: *mut sunrealtype,
    user_data: *mut c_void,
) -> i32 {
    unsafe {
        if user_data.is_null() {
            return -1;
        }

        let ode: &mut T = &mut *(user_data as *mut T);

        let y_slice = (*y).as_mut();
        let gout_slice = from_raw_parts_mut(gout, ode.nz());

        ode.g(t, y_slice, gout_slice).map_or(-1, |_| 0)
    }
}

// Jacobian function
extern "C" fn jac<T: Ode>(
    t: sunrealtype,
    y: N_Vector,
    _fy: N_Vector,
    Jac: SUNMatrix,
    user_data: *mut std::ffi::c_void,
    _tmp1: N_Vector,
    _tmp2: N_Vector,
    _tmp3: N_Vector,
) -> i32 {
    if user_data.is_null() {
        return -1;
    }

    unsafe {
        let ode: &mut T = &mut *(user_data as *mut T);
        let y = (*y).as_mut();
        let J = (*Jac).as_mut();
        ode.jacobian(t, y, J).map_or(-1, |_| 0)
    }
}
