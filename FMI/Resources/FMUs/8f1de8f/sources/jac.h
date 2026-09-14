/* Internal header file for Jacobian computations. */

#ifndef jac_h
#define jac_h

#include "types.h"

/* ----------------- function delcarations ----------------- */

/* Setup */
DYMOLA_STATIC void jac_setup(size_t ngroups, FMIBoolean ida, size_t cgoffset, size_t gcoffset);

/* computes function f(t,y) */
DYMOLA_STATIC int jac_f(realtype t, N_Vector y, N_Vector ydot, void *user_data);

#ifdef INCLUDE_SUNDIALS_IDA
/* computes residual f(t,y) - y' */
DYMOLA_STATIC int jac_res_IDA(realtype t, N_Vector y, N_Vector ydot, N_Vector res, void *user_data);
#endif

/* Jacobian routine, computes J(t,y) = df/dy numerically */
DYMOLA_STATIC int jac_Jacobian(realtype t,
							   N_Vector y, N_Vector fy, SUNMatrix J, void *user_data,
							   N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
/* sparse Jacobian routine, computes J(t,y) = df/dy numerically */
DYMOLA_STATIC int jac_JacobianSparse(realtype t,
							   N_Vector y, N_Vector fy, SUNMatrix J, void *user_data,
							   N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#endif

#ifdef INCLUDE_SUNDIALS_IDA
/* Jacobian routine, computes DAE Jacobian */
DYMOLA_STATIC int jac_Jacobian_IDA(
	realtype t, realtype c_j, N_Vector y, N_Vector ydot, N_Vector res, SUNMatrix J, void *user_data,
	N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#endif

#endif /* jac_h */
