#ifndef ONLY_INCLUDE_INLINE_INTEGRATION

#include "ds_sundials_extensions/ds_sundials_extensions.h"
#include "cvode_impl.h"
#include "sunmatrix/sunmatrix_dense.h"

/* Step Root functionality Added by 3ds.com */
SUNDIALS_EXPORT int DSCVodeStepRootInit(void *cvode_mem, CVStepRootFn g) 
{
  CVodeMem cv_mem;

  /* Check cvode_mem pointer */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeRootInit", MSGCV_NO_MEM);
    return(CV_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  cv_mem->cv_stepgfun = g;
  return(CV_SUCCESS);
}

/* Modify the CVode Nordsieck history array to handle discontinuous changes in the second derivatiave of the state, xbis. Here xbis_update is 
   the difference between the value of xbis after, compared to before the discontinuity. */
SUNDIALS_EXPORT int DSCVodeUpdateNordsieckXbis(void* cvode_mem, const N_Vector xbis_update)
{
  CVodeMem cv_mem;

  /* Check cvode_mem pointer */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CV_MEM_NULL, "CVODE", "DSCVodeUpdateNordsieckXbis", MSGCV_NO_MEM);
    return(CV_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  if (cv_mem->cv_qu >= 2 && cv_mem->cv_next_q >= 2)
	N_VLinearSum(1.0, cv_mem->cv_zn[2], cv_mem->cv_hu * cv_mem->cv_hu / 2.0, xbis_update, cv_mem->cv_zn[2]);

  return(CV_SUCCESS);
}

/* For Simulator Performance (Simulation Analysis) integrator state error logging */
SUNDIALS_EXPORT int DSCVodeSetStateErrorMonitorFn(void* cvode_mem, CVodeStateErrorMonitorFN fn)
{
  CVodeMem cv_mem;

  /* Check cvode_mem pointer */
  if (cvode_mem == NULL) {
      cvProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeSetStateErrorMonitorFn", MSGCV_NO_MEM);
      return(CV_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  cv_mem->cv_stateerrormonitorfun = fn;
  return (CV_SUCCESS);
}

#if defined(INCLUDE_SUNDIALS_IDA) || defined(GODESS)

#undef ETA_MAX_FX_DEFAULT
#undef ETA_MIN_FX_DEFAULT
#undef ETA_MIN_DEFAULT
#undef ETA_MIN_EF_DEFAULT
#undef MSG_TIME
#undef MSG_TIME_H
#include "ida_impl.h"

/* Step Root functionality Added by 3ds.com */
SUNDIALS_EXPORT int DSIDAStepRootInit(void *IDA_mem, IDAStepRootFn g) 
{
  IDAMem ida_mem;

  /* Check cvode_mem pointer */
  if (IDA_mem == NULL) {
    IDAProcessError(NULL, IDA_MEM_NULL, "IDA", "IDARootInit", MSG_NO_MEM);
    return(IDA_MEM_NULL);
  }
  ida_mem = (IDAMem) IDA_mem;

  ida_mem->ida_stepgfun = g;
  return(IDA_SUCCESS);
}

/* IDA monitor function mimicing CVode monitor function */
SUNDIALS_EXPORT int DSIDASetMonitorFn(void *IDA_mem, IDAMonitorFN fn)
{
  IDAMem ida_mem;

  if (IDA_mem==NULL) {
    IDAProcessError(NULL, IDA_MEM_NULL, "IDA", "IDASetMonitorFn", MSG_NO_MEM);
    return(IDA_MEM_NULL);
  }

  ida_mem = (IDAMem) IDA_mem;

#ifdef SUNDIALS_BUILD_WITH_MONITORING
  ida_mem->ida_monitorfun = fn;
  return(IDA_SUCCESS);
#else
  IDAProcessError(ida_mem, IDA_ILL_INPUT, "IDA", "IDASetMonitorFn", "SUNDIALS was not built with monitoring enabled.");
  return(IDA_ILL_INPUT);
#endif
}

/* Set function to communicate when accurate DAE events are required. Setting the function enables the functionality. Set to null to disable */
SUNDIALS_EXPORT int DSIDASetAccurateDAEEventFn(void *IDA_mem, IDAAtDAEEvent fn) 
{
  IDAMem ida_mem;

  /* Check cvode_mem pointer */
  if (IDA_mem == NULL) {
    IDAProcessError(NULL, IDA_MEM_NULL, "IDA", "DSIDASetAccurateDAEEventFn", MSG_NO_MEM);
    return(IDA_MEM_NULL);
  }
  ida_mem = (IDAMem) IDA_mem;

  ida_mem->ida_atdaeeventfun = fn;
  return(IDA_SUCCESS);
}

/* Modify the IDA history arrays to handle discontinuous changes in the second derivatiave of the state, xbis. Here xbis_update is the 
   difference between the value of xbis after, compared to before the discontinuity. */
SUNDIALS_EXPORT int DSIDAUpdateHistoryXbis(void* IDA_mem, const N_Vector xbis_update)
{
  IDAMem ida_mem;

  /* Check cvode_mem pointer */
  if (IDA_mem == NULL) {
    IDAProcessError(NULL, IDA_MEM_NULL, "IDA", "DSIDAUpdateHistoryXbis", MSG_NO_MEM);
    return(IDA_MEM_NULL);
  }
  ida_mem = (IDAMem) IDA_mem;

  if (ida_mem->ida_kk >= 2) {
	  const double hn = ida_mem->ida_psi[0];
	  const double hn_p_hnm1 = ida_mem->ida_psi[1];

	  /* The second derivative xbis only shows up in phi[1] and phi[2] and how it shows up there is known.
	     The idea is to compute the same directional derivative as for CVode and then just correct phi[1] and phi[2]:
			phi[1] = phi[1] - hn^2/2.0 * xbis_update
			phi[2] = phi[2] + hn*(hn+hnm1)/2.0 * xbis_update */
	  N_VLinearSum(1.0, ida_mem->ida_phi[1], -hn*hn/2.0, xbis_update, ida_mem->ida_phi[1]);
	  N_VLinearSum(1.0, ida_mem->ida_phi[2], hn*hn_p_hnm1/2.0, xbis_update, ida_mem->ida_phi[2]);
  }

  return(IDA_SUCCESS);
}

/* For Simulator Performance (Simulation Analysis) integrator state error logging */
SUNDIALS_EXPORT int DSIDASetStateErrorMonitorFn(void* IDA_mem, IDAStateErrorMonitorFN fn)
{
    IDAMem ida_mem;

    if (IDA_mem == NULL) {
        IDAProcessError(NULL, IDA_MEM_NULL, "IDA", "DSIDASetStateErrorMonitorFn", MSG_NO_MEM);
        return(IDA_MEM_NULL);
    }

    ida_mem = (IDAMem) IDA_mem;

    ida_mem->ida_stateerrormonitorfun = fn;
    return(IDA_SUCCESS);
}

/* Functions with dense and sparse variants */

SUNDIALS_EXPORT int DSSUNMatAddDiagonal_Dense(realtype val, SUNMatrix A)
{
	sunindextype i;

	if (SM_COLUMNS_D(A) != SM_ROWS_D(A))
		return SUNMAT_ILL_INPUT;

	/* Perform operation A = A + val*I */
	for (i = 0; i < SM_COLUMNS_D(A); i++)
		SM_ELEMENT_D(A, i, i) += val;

	return SUNMAT_SUCCESS;
}

SUNDIALS_EXPORT int DSSUNMatAddPartialDiagonal_Dense(realtype val, sunindextype nxorig, SUNMatrix* partialIPtr, SUNContext sunctx, SUNMatrix A)
{
	sunindextype i;

	if (SM_COLUMNS_D(A) != SM_ROWS_D(A) || nxorig < 0 || nxorig > SM_COLUMNS_D(A))
		return SUNMAT_ILL_INPUT;

	/* Perform operation A = A + val*{{I,0},{0,0}} */
	for (i = 0; i < nxorig; i++)
		SM_ELEMENT_D(A, i, i) += val;

	return SUNMAT_SUCCESS;
}

#endif
#endif
