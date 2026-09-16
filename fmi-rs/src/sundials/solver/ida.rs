use std::ffi::c_void;
use std::slice::from_raw_parts_mut;

use crate::fmi3::types::{fmi3Status, fmi3ValueReference};
use crate::sim::SimulationError;
use crate::sim::solver::{Dae, Ode, Solver, SolverFactory};
use crate::sundials::ida::{
    IDA_NORMAL, IDA_ROOT_RETURN, IDA_SUCCESS, IDA_TSTOP_RETURN, IDACreate, IDAFree, IDAInit,
    IDAReInit, IDARootInit, IDASVtolerances, IDASetUserData, IDASolve,
};
use crate::sundials::ida_ls::{IDASetJacFn, IDASetLinearSolver};
use crate::sundials::nvector_serial::N_VNew_Serial;
use crate::sundials::sundials_context::{SUNContext_Create, SUNContext_Free};
use crate::sundials::sundials_linearsolver::{SUNLinSolFree, SUNLinearSolver};
use crate::sundials::sundials_matrix::{SUNMatDestroy, SUNMatrix};
use crate::sundials::sundials_nvector::{N_VDestroy, N_Vector};
use crate::sundials::sundials_types::{SUN_COMM_NULL, SUNContext, sunrealtype};
use crate::sundials::sunlinsol_dense::SUNLinSol_Dense;
use crate::sundials::sunmatrix_dense::SUNDenseMatrix;
use crate::{fmi3::FMU3, sim::fmi3::input::StaticInput};

pub struct Ida<D: Dae> {
    sunctx: SUNContext,
    yy: N_Vector,
    yp: N_Vector,
    rtol: sunrealtype,
    avtol: N_Vector,
    A: SUNMatrix,
    LS: SUNLinearSolver,
    ida_mem: *mut c_void,
    dae: Box<D>,
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

macro_rules! expect_ok {
    ($result:expr) => {
        if $result != fmi3Status::Ok {
            return Err(SimulationError::FMICall);
        }
    };
}

extern "C" fn res<D: Dae>(
    tt: sunrealtype,
    yy: N_Vector,
    yp: N_Vector,
    rr: N_Vector,
    user_data: *mut c_void,
) -> i32 {
    if user_data.is_null() {
        return -1;
    }

    unsafe {
        let dae: &D = &*(user_data as *const D);
        dae.residuals(tt, (*yy).as_mut(), (*yp).as_mut(), (*rr).as_mut())
            .map_or(-1, |_| 0)
    }
}

extern "C" fn g<D: Dae>(
    t: sunrealtype,
    yy: N_Vector,
    _yp: N_Vector,
    gout: *mut sunrealtype,
    user_data: *mut c_void,
) -> i32 {
    if user_data.is_null() {
        return -1;
    }

    unsafe {
        let dae: &D = &*(user_data as *const D);
        let knowns = (*yy).as_mut();
        let gout_slice = from_raw_parts_mut(gout, dae.nz());
        dae.root(t, knowns, gout_slice).map_or(-1, |_| 0)
    }
}

extern "C" fn jac<D: Dae>(
    tt: sunrealtype,
    cj: sunrealtype,
    yy: N_Vector,
    _yp: N_Vector,
    _resvec: N_Vector,
    JJ: SUNMatrix,
    user_data: *mut c_void,
    _tmp1: N_Vector,
    _tmp2: N_Vector,
    _tmp3: N_Vector,
) -> i32 {
    if user_data.is_null() {
        return -1;
    }

    unsafe {
        let dae: &D = &*(user_data as *const D);
        let y = (*yy).as_mut();
        let J = (*JJ).as_mut();
        dae.jacobian(tt, y, cj, J).map_or(-1, |_| 0)
    }
}

impl<D: Dae> Solver for Ida<D> {
    fn step(&mut self, next_time: f64) -> Result<(f64, &[f64], bool), SimulationError> {
        unsafe {
            let mut tret = 0.0;

            let retval = IDASolve(
                self.ida_mem,
                next_time,
                &mut tret,
                self.yy,
                self.yp,
                IDA_NORMAL,
            );

            if retval == IDA_TSTOP_RETURN {
                return Err(SimulationError::Solver("IDA_TSTOP_RETURN".to_owned()));
            }

            if retval < IDA_SUCCESS {
                return Err(SimulationError::Solver(format!(
                    "IDASolve failed with code {retval}"
                )));
            }

            Ok((next_time, (*self.yy).as_mut(), retval == IDA_ROOT_RETURN))
        }
    }

    fn reset(&mut self, time: f64) -> Result<(), SimulationError> {
        unsafe {
            let knowns = (*self.yy).as_mut();
            let absolut_tolerances = (*self.avtol).as_mut();
            let unknowns = (*self.yp).as_mut();

            self.dae.init(knowns, absolut_tolerances, unknowns)?;

            for absolut_tolerance in absolut_tolerances.iter_mut() {
                *absolut_tolerance *= self.rtol;
            }

            expect_no_error!(IDAReInit(self.ida_mem, time, self.yy, self.yp), "");

            expect_no_error!(
                IDASVtolerances(self.ida_mem, self.rtol, self.avtol),
                "Failed to set tolerances"
            );
        }

        Ok(())
    }
}

impl<D: Dae> Drop for Ida<D> {
    fn drop(&mut self) {
        unsafe {
            IDAFree(&mut self.ida_mem);
            SUNLinSolFree(self.LS);
            N_VDestroy(self.yy);
            N_VDestroy(self.yp);
            N_VDestroy(self.avtol);
            SUNMatDestroy(self.A);
            SUNContext_Free(&mut self.sunctx);
        }
    }
}

pub struct IdaSolverFactory;

impl SolverFactory for IdaSolverFactory {
    fn create<'a, O: Ode + 'a, D: Dae + 'a>(
        &self,
        start_time: f64,
        rtol: f64,
        _ode: O,
        dae: Option<D>,
    ) -> Result<Box<dyn Solver + 'a>, SimulationError> {
        let dae = dae.ok_or_else(|| SimulationError::Solver("No DAE provided".to_owned()))?;
        let neq = dae.neq() as i64;

        unsafe {
            let mut sunctx = std::ptr::null_mut();

            expect_no_error!(
                SUNContext_Create(SUN_COMM_NULL, &mut sunctx),
                "Failed to create SUNDIALS context"
            );

            let ida_mem = IDACreate(sunctx);
            expect_not_null!(ida_mem, "Failed to create IDA memory");

            // Allocate N-vectors
            let yy: *mut crate::sundials::sundials_nvector::_generic_N_Vector =
                N_VNew_Serial(neq, sunctx);
            expect_not_null!(yy, "Failed to create yy vector");

            let yp = N_VNew_Serial(neq, sunctx);
            expect_not_null!(yp, "Failed to create yp vector");

            let avtol = N_VNew_Serial(neq, sunctx);
            expect_not_null!(avtol, "Failed to create avtol vector");

            let A = SUNDenseMatrix(neq, neq, sunctx);
            expect_not_null!(A, "Failed to create A matrix");

            // Initialize vectors
            let nominals = (*avtol).as_mut();
            dae.init((*yy).as_mut(), nominals, (*yp).as_mut())?;

            for nominal in nominals.iter_mut() {
                *nominal *= rtol;
            }

            expect_no_error!(
                IDAInit(ida_mem, res::<D>, start_time, yy, yp),
                "Failed to initilalize IDA"
            );

            if dae.nz() > 0 {
                expect_no_error!(
                    IDARootInit(ida_mem, dae.nz() as i32, g::<D>),
                    "Failed to set root function"
                );
            }

            expect_no_error!(
                IDASVtolerances(ida_mem, rtol, avtol),
                "Failed to set tolerances"
            );

            let LS = SUNLinSol_Dense(yy, A, sunctx);
            expect_not_null!(LS, "Failed to create dense SUNLinearSolver");

            expect_no_error!(
                IDASetLinearSolver(ida_mem, LS, A),
                "Failed to attach the matrix and linear solver"
            );

            expect_no_error!(
                IDASetJacFn(ida_mem, jac::<D>),
                "Failed to set Jacobian routine"
            );

            let dae = Box::new(dae);

            let user_data: *const D = &*dae;

            expect_no_error!(
                IDASetUserData(ida_mem, user_data as *mut c_void),
                "Failed to set user data"
            );

            let ida = Ida {
                sunctx,
                yy,
                yp,
                rtol,
                avtol,
                A,
                LS,
                ida_mem,
                dae,
            };

            Ok(Box::new(ida))
        }
    }
}
