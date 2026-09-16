#![allow(non_camel_case_types, non_snake_case)]

use std::{ffi::c_void, slice::from_raw_parts_mut};

use fmi_rs::sundials::{
    ida::{
        IDA_NORMAL, IDA_ROOT_RETURN, IDA_SUCCESS, IDA_TSTOP_RETURN, IDACreate, IDAFree,
        IDAGetRootInfo, IDAInit, IDARootInit, IDASVtolerances, IDASetNonlinearSolver, IDASolve,
    },
    ida_ls::{IDASetJacFn, IDASetLinearSolver},
    nvector_serial::N_VNew_Serial,
    sundials_context::{SUNContext_Create, SUNContext_Free},
    sundials_linearsolver::SUNLinSolFree,
    sundials_matrix::{SUNMatDestroy, SUNMatrix},
    sundials_nonlinearsolver::SUNNonlinSolFree,
    sundials_nvector::{N_VDestroy, N_Vector},
    sundials_types::{SUN_COMM_NULL, sunrealtype},
    sunlinsol_dense::SUNLinSol_Dense,
    sunmatrix_dense::{SM_ELEMENT_D, SUNDenseMatrix},
    sunnonlinsol_newton::SUNNonlinSol_Newton,
};
use rstest::*;

// #define IJth(A, i, j) SM_ELEMENT_D(A, i - 1, j - 1)
fn IJth(A: SUNMatrix, i: usize, j: usize) -> *mut sunrealtype {
    SM_ELEMENT_D(A, i - 1, j - 1)
}

extern "C" fn resrob(
    _tt: sunrealtype,
    yy: N_Vector,
    yp: N_Vector,
    rr: N_Vector,
    _user_data: *mut c_void,
) -> i32 {
    unsafe {
        let yval = (*yy).as_mut();
        let ypval = (*yp).as_mut();
        let rval = (*rr).as_mut();

        rval[0] = -0.04 * yval[0] + 1.0e4 * yval[1] * yval[2];
        rval[1] = -rval[0] - 3.0e7 * yval[1] * yval[1] - ypval[1];
        rval[0] -= ypval[0];
        rval[2] = yval[0] + yval[1] + yval[2] - 1.0;
    }
    0
}

extern "C" fn grob(
    _t: sunrealtype,
    yy: N_Vector,
    _yp: N_Vector,
    gout: *mut sunrealtype,
    _user_data: *mut c_void,
) -> i32 {
    unsafe {
        let yval: &mut [f64] = (*yy).as_mut();
        let y1 = yval[0];
        let y3 = yval[2];
        let gout_slice = from_raw_parts_mut(gout, 2usize);
        gout_slice[0] = y1 - 0.0001;
        gout_slice[1] = y3 - 0.01;
    }
    0
}

// Define the Jacobian function.
extern "C" fn jacrob(
    _tt: sunrealtype,
    cj: sunrealtype,
    yy: N_Vector,
    _yp: N_Vector,
    _resvec: N_Vector,
    JJ: SUNMatrix,
    _user_data: *mut c_void,
    _tmp1: N_Vector,
    _tmp2: N_Vector,
    _tmp3: N_Vector,
) -> i32 {
    unsafe {
        let yval = (*yy).as_mut();

        *IJth(JJ, 1, 1) = -0.04 - cj;
        *IJth(JJ, 2, 1) = 0.04;
        *IJth(JJ, 3, 1) = 1.0;
        *IJth(JJ, 1, 2) = 1.0e4 * yval[2];
        *IJth(JJ, 2, 2) = -1.0e4 * yval[2] - 6.0e7 * yval[1] - cj;
        *IJth(JJ, 3, 2) = 1.0;
        *IJth(JJ, 1, 3) = 1.0e4 * yval[1];
        *IJth(JJ, 2, 3) = -1.0e4 * yval[1];
        *IJth(JJ, 3, 3) = 1.0;
    }
    0
}

#[rstest]
fn test_ida() {
    let NEQ = 3;
    let NOUT = 12;

    unsafe {
        // Create SUNDIALS context
        let mut ctx = std::ptr::null_mut();
        let err_code = SUNContext_Create(SUN_COMM_NULL, &mut ctx);

        assert!(
            err_code == 0,
            "Failed to create SUNDIALS context: error code {}",
            err_code
        );

        // Allocate N-vectors
        let yy = N_VNew_Serial(NEQ, ctx);
        let yp = N_VNew_Serial(NEQ, ctx);
        let avtol = N_VNew_Serial(NEQ, ctx);

        // Create and initialize  y, y', and absolute tolerance vectors
        let yval = (*yy).as_mut();
        yval[0] = 1.0;
        yval[1] = 0.0;
        yval[2] = 0.0;

        let ypval = (*yp).as_mut();
        ypval[0] = -0.04;
        ypval[1] = 0.04;
        ypval[2] = 0.0;

        let rtol = 1.0e-4;

        let atval = (*avtol).as_mut();
        atval[0] = 1.0e-8;
        atval[1] = 1.0e-6;
        atval[2] = 1.0e-6;

        // Integration limits
        let t0 = 0.0;
        let tout1 = 0.4;

        // Call IDACreate and IDAInit to initialize IDA memory
        let mut mem = IDACreate(ctx);
        assert!(!mem.is_null(), "Failed to create IDA memory");

        let retval = IDAInit(mem, resrob, t0, yy, yp);
        assert!(
            retval == 0,
            "Failed to initialize IDA: error code {}",
            retval
        );

        // Call IDASVtolerances to set tolerances
        let retval = IDASVtolerances(mem, rtol, avtol);
        assert!(
            retval == 0,
            "Failed to set tolerances: error code {}",
            retval
        );

        // Call IDARootInit to specify the root function grob with 2 components
        let retval = IDARootInit(mem, 2, grob);
        assert!(
            retval == 0,
            "Failed to set root function: error code {}",
            retval
        );

        // Create dense SUNMatrix for use in linear solves
        let A = SUNDenseMatrix(NEQ, NEQ, ctx);
        assert!(retval == 0, "SUNDenseMatrix failed: error code {}", retval);

        // Create dense SUNLinearSolver object
        let LS = SUNLinSol_Dense(yy, A, ctx);
        assert!(!LS.is_null(), "Failed to create dense SUNLinearSolver");

        // Attach the matrix and linear solver
        let retval = IDASetLinearSolver(mem, LS, A);
        assert!(
            retval == 0,
            "IDASetLinearSolver failed: error code {}",
            retval
        );

        // Set the user-supplied Jacobian routine
        let retval = IDASetJacFn(mem, jacrob);
        assert!(retval == 0, "IDASetJacFn failed: error code {}", retval);

        // Create Newton SUNNonlinearSolver object. IDA uses a
        // Newton SUNNonlinearSolver by default, so it is unnecessary
        // to create it and attach it. It is done in this example code
        // solely for demonstration purposes.
        let NLS = SUNNonlinSol_Newton(yy, ctx);
        assert!(
            retval == 0,
            "SUNNonlinSol_Newton failed: error code {}",
            retval
        );

        // Attach the nonlinear solver
        let retval = IDASetNonlinearSolver(mem, NLS);
        assert!(
            retval == 0,
            "IDASetNonlinearSolver failed: error code {}",
            retval
        );

        // In loop, call IDASolve, print results, and test for error.
        // Break out of loop when NOUT preset output times have been reached.

        let mut iout = 0;
        let mut tout = tout1;

        loop {
            let mut tret = 0.0;

            let retval = IDASolve(mem, tout, &mut tret, yy, yp, IDA_NORMAL);

            if retval == IDA_TSTOP_RETURN {
                break;
            }

            if retval == IDA_ROOT_RETURN {
                let mut rootsfound = [0; 2];

                let retvalr = IDAGetRootInfo(mem, rootsfound.as_mut_ptr());
                assert!(retvalr == 0, "IDAGetRootInfo failed: error code {}", retval);

                eprintln!("rootsfound: {rootsfound:?}");
            }

            if retval == IDA_SUCCESS {
                iout += 1;
                tout *= 10.0;
            }

            if iout == NOUT {
                break;
            }

            eprintln!("{tout} {:?}", (*yy).as_mut());
        }

        // Free memory
        IDAFree(&mut mem);
        SUNNonlinSolFree(NLS);
        SUNLinSolFree(LS);
        SUNMatDestroy(A);
        N_VDestroy(avtol);
        N_VDestroy(yy);
        N_VDestroy(yp);
        SUNContext_Free(&mut ctx);
    }
}
