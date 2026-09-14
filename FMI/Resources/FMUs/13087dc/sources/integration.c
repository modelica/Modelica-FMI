/* Implementation of integration.h */

#include "conf.h"
#include "util.h"
#include "integration.h"
#include "jac.h"
#include "result.h"
#include "cexch.h"

#include "fmiFunctions_fwd.h"

#include "GenerateResultInNonDymosim.h"

/* Sundials */
#include <cvode/cvode.h>
#include <sunlinsol/sunlinsol_dense.h>
#include "ds_sundials_extensions/ds_sundials_extensions.h"
#include <sundials/sundials_math.h>
#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
#include <sunmatrix/sunmatrix_sparse.h>
#endif

/* for Dymola_abs */
#include "f2c.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#include "adymosim.h"

/* ----------------- constants ----------------- */

#ifndef DYMOLA_STATIC_IMPORT
#define DYMOLA_STATIC_IMPORT DYMOLA_STATIC
#endif

static FMIBoolean IDA = FMIFalse;
extern const double cvodeTolerance;

#define RTOL_DEFAULT (1.0e-5)   /* scalar relative tolerance */

/* when testing with Modelica wrapper, the external step size may be large due to
   variable step size solvers being used externally */
#define MAX_STEPS_BEFORE_REINIT 5000	/* maximum number of internal steps before first call to reinit */
#define MAX_STEPS_TOTAL 100000			/* maximum number if internal steps for a time step */
#define STEPS_BEFORE_REINIT_FACTOR 3	/* multiplier for number of internal steps to allow before next reinit */
#define MAX_FAILURES 15       /* maximum number of error test failures permitted in attempting one step */
#define MAX_NON_LIN_ITERS 3   /* maximum number of non-linear iterations */

/* for Sundials */
/* the safety factor used in the nonlinear convergence test */
#define NON_LINEAR_CONV_TEST_COEFF 1e-1

/* ----------------- macros ----------------- */

#define CALL_SET(function, val)                               \
    flag = function(cvode_mem, val);                          \
	if (util_check_flag(&flag, #function, 1, comp)) goto fail \

#define CALL_SET2(function, val1, val2)                       \
    flag = function(cvode_mem, val1, val2);                   \
    if (util_check_flag(&flag, #function, 1, comp)) goto fail \

#define GET_STATISTIC(function, var)                          \
	flag = function(comp->iData->cvode_mem, &var);            \
	if (util_check_flag(&flag, #function, 1, comp)) {         \
	  var = -1;                                               \
	}

#define INT_RESULT_SAMPLE(atEvent)                            \
if (comp->storeResult == FMITrue) {                           \
		result_sample(comp, atEvent);                         \
}

/* ----------------- local function declarations ----------------- */

/* clones integration data */
static IntegrationStatus clone_data(Component* target, Component* source);

/* clones interpolation data */
static FMIStatus clone_ipData(Component* target, Component* source);

/* frees integration data */
static IntegrationStatus free_data(Component* comp, Component* target);

/* root function used for event indicators */
static int g(realtype t, N_Vector y, realtype *gout, void *user_data);

#ifdef INCLUDE_SUNDIALS_IDA
/* root function used for event indicators */
static int g_IDA(realtype t, N_Vector y, N_Vector ydot, realtype * gout, void* user_data);
#endif

/* step event check function */
static int check_step_event(void* user_data);

/* error message handler */
static void err_msg_handler(int error_code, const char *module,
							const char *function, char *msg,
							void *eh_data);

/* sets the weights for the weighted norm */
static int cvEwtSetVV(N_Vector y, N_Vector ewt, void *user_data);

/* checks if a steady state has been reached */
static int steady_state_reached(Component* comp);

/* checks if the embedded max run time has been reached */
static int max_run_time_reached(Component* comp);

/* initialize SparseFixData struct */
static void InitializeSparseFixData(SparseFixData* sparseData);

/* copy SparseFixData struct */
static void CopySparseFixData(const size_t nStates, SparseFixData* target, const SparseFixData* source);

#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
/* checks if sparse solver is to be used and returns number of nonzero elements */
static int EnableSparse(int nx, SparseFixData *sparseData, int cgOffset, int gcOffset);

/* computes fixed Jacobian structure data to be used when calculating numeric Jacobian using grouping */
static int PrepareSparse(Component *comp, SparseFixData *sparseData, int N, int nz, int cgOffset, int gcOffset);

/* return the number of cores to be used by the multithreaded sparse SuperLU */
static int getNumProcsCvode(SparseFixData *sparseData, int nx, int nnz, int bw);

/* Allocate memory needed for sparse analytic Jacobian construction */
static int InitializeSparseAnalyticData(IntegrationData* iData, const int num_states, const int num_non_zero);

/* Copy memory needed for sparse analytic Jacobian construction */
static void CopySparseAnalyticData(IntegrationData* iData_target, IntegrationData* iData_source);

/* De-allocate memory needed for sparse analytic Jacobian construction */
static void FreeSparseAnalyticData(IntegrationData* iData);
#endif

/* Called by CVode after each successful step. Gives access to the internal memory structure for logging.
   Do not modify the CVode memory as leads to undefined behaviour. */
static int MonitorFn(void* cvode_mem, void* user_data);

/* Enables 3ds accurate DAE event handling */
static int SetAccurateDAECrossing(void* user_data, int accurateCrossing);

/* For Simulator Performance (Simulation Analysis) integrator state error logging */
static void StateErrorMonitorFn(void* user_data, realtype time, realtype h, int q, int rejected,
	sunindextype N, realtype* ewt, realtype* e, realtype err, realtype mult);

/* update offsets for QJacobian for DAE mode */
static void UpdateOffset(int* const cgOffset, int* const gcOffset);

/* ----------------- local variables ----------------- */

static size_t nGroups;

/* when compiling as a single complilation unit, this is already defined */
#ifndef FMU_SOURCE_SINGLE_UNIT
extern Component* globalComponent2;
#endif

/* ----------------- external variables ----------------- */

extern int QJacobianCG_[];
extern struct QJacobianTag_ QJacobianGC2_[];
extern int QJacobianDefined_;

/* ----------------- external function declarations ----------------- */

extern void initializeData2(double* t);
DYMOLA_STATIC void DYNPropagateDidToThread(struct DYNInstanceData* did);
DYMOLA_STATIC int GetDaeIterationVariableNominals(const struct DYNInstanceData* const did_, double* const dae_iteration_variable_nominals);
extern void declareNew2_(double*, double*, double*, double*, double*, double*, double*, void**, int*, int, struct DeclarePhase*);
void updateErrCount3(doublereal time, doublereal step_size, int order, int rejected, int neq, const doublereal*wt, const doublereal*e, doublereal err, doublereal mult, int invW, doublereal start_time, char const** xNames);
void statisticsErrCount3(doublereal time, int not_supported, doublereal start_time, char const** xNames);
extern void errCountAllocate(int size);
extern void errCountDeAllocate();


/* ----------------- function definitions ----------------- */

/* ------------------------------------------------------ */
DYMOLA_STATIC IntegrationStatus integration_setup(FMIComponent c, FMIReal relTol)
{
	Component* comp = (Component*) c;
	realtype *abstol_data, *reltol_data;
	void *cvode_mem;
	int flag;
	size_t i;
	int cgOffset = 0;
	int gcOffset = 0;

	IntegrationData* iData;
	
	assert(comp->iData == NULL);
	assert(sizeof(realtype) == sizeof(FMIReal));

	if (comp->nStates == 0) {
		/* Add dummy state fo simplify code and avoid extra conditions for the vast majority of models.
		Note that concerned arrays already have 1 allocated element due to guarding offset 1 during instantiation. */
		comp->nStates = 1;
		comp->nxOrig = 1;
		comp->states[0] = 0;
		comp->statesNominal[0] = 1;
		if (comp->useInputSmoother) { // Shouldn't happen
			util_logger(comp, comp->instanceName, FMIError, loggSundials,
				"Input smoother is not available when there are no states.");
		}
	}

	if (relTol <= 0) {
		relTol = RTOL_DEFAULT;
	}
	comp->iData = (IntegrationData*) ALLOCATE_MEMORY(1, sizeof(IntegrationData));
	if (comp->iData == NULL) goto fail;

	iData = comp->iData;

	iData->y = iData->yDot = iData->abstol = iData->reltol = iData->ewt = iData->tmp1 = iData->tmp2 = iData->tmp3 = NULL;
	iData->delta_udot = iData->df_du_delta_udot = NULL;
	iData->f_vrefs = iData->u_vrefs = NULL;
#if (FMI_VERSION >= FMI_2)
	iData->save_state = NULL;
#endif
	iData->cvode_mem = NULL;

	InitializeSparseFixData(&iData->sparseData);
	iData->sparseAnalyticData = NULL;

	iData->steadyStateUtils = NULL;

	iData->sunctx = NULL;
	iData->A = iData->partialI = NULL;
	iData->linearSolver = NULL;
	iData->DAEMinorEventRestart = FMIFalse;
	iData->DAEMinorEventRestartTime = 0.0;
	SUNContext_Create(NULL, &iData->sunctx);
	if (util_check_flag((void*)iData->sunctx, "SUNContext_Create", 0, comp)) goto fail;

	/* create serial vector from states */
	assert(comp->nStates > 0);
	/* it turns out using N_VNew_Serial(comp->nStates) gives worse accuracy for some reason */
	iData->y = N_VMake_Serial((long)comp->nStates, comp->states, iData->sunctx);
	if (util_check_flag((void *)(iData->y), "N_VMake_Serial", 0, comp)) goto fail;

	/* Backward Differentiation Formula and the use of a Newton iteration */
	IDA = (FMIBoolean) GetAdditionalFlags(27);
#ifdef INCLUDE_SUNDIALS_IDA
	if (IDA) {
		cvode_mem = IDACreate(iData->sunctx);
	} else 
#endif
	{
		if (IDA) {
			util_logger(comp, comp->instanceName, FMIError, loggSundials, "SUNDIALS_ERROR: The solver IDA was selected but the IDA sources were not included in the build.");
			goto fail;
		}

		cvode_mem = CVodeCreate(CV_BDF, iData->sunctx);
	}
	if (util_check_flag((void *)cvode_mem, "CVodeCreate", 0, comp)) goto fail;

	/* initialize the integrator memory and specify the
	   user's right hand side function in y'=f(t,y), the inital time and
	   the initial dependent variable vector y */
#ifdef INCLUDE_SUNDIALS_IDA
	if (IDA) {
		iData->yDot = N_VMake_Serial((long)comp->nStates, comp->derivatives, iData->sunctx);
		if (util_check_flag((void*)(iData->yDot), "N_VMake_Serial", 0, comp)) goto fail;
		flag = IDAInit(cvode_mem, jac_res_IDA, comp->time, iData->y, iData->yDot);
	} else 
#endif
	{
		flag = CVodeInit(cvode_mem, jac_f, comp->time, iData->y);
	}
	if (util_check_flag(&flag, "CVodeInit", 1, comp)) goto fail;

	/* need to provide the context from FMI */
#ifdef INCLUDE_SUNDIALS_IDA
	if (IDA) {
		CALL_SET(IDASetUserData, comp);
		CALL_SET(DSIDASetMonitorFn, MonitorFn);
	} else 
#endif
	{
		CALL_SET(CVodeSetUserData, comp);
		CALL_SET(CVodeSetMonitorFn, MonitorFn);
		CALL_SET(CVodeSetMonitorFrequency, 1);
	}

	/* specify the relative and absolute tolerances */
	iData->abstol = N_VNew_Serial((long)comp->nStates, iData->sunctx);
	if (util_check_flag((void *)(iData->abstol), "N_VNew_Serial", 0, comp)) goto fail;
	iData->reltol = N_VNew_Serial((long)comp->nStates, iData->sunctx);
	if (util_check_flag((void *)(iData->reltol), "N_VNew_Serial", 0, comp)) goto fail;
	abstol_data = N_VGetArrayPointer(iData->abstol);
	reltol_data = N_VGetArrayPointer(iData->reltol);
	for (i = 0; i < comp->nxOrig; i++) {
		realtype nom = comp->statesNominal[i];
		if (nom == 0) nom = 1;
		if (nom >= 0) {
			abstol_data[i] = relTol * nom;
			reltol_data[i] = relTol;
		} else {
			abstol_data[i] = relTol * (-nom);
			reltol_data[i] = 0.0;
		}
	}
#ifdef INCLUDE_SUNDIALS_IDA
	if (comp->DAEMode) {
		GetDaeIterationVariableNominals(comp->did, reltol_data + i);
		for (; i < comp->nStates; i++) {
			realtype nom = reltol_data[i];
			if (nom == 0) nom = 1;
			if (nom >= 0) {
				abstol_data[i] = relTol * nom;
				reltol_data[i] = relTol;
			} else {
				abstol_data[i] = relTol * (-nom);
				reltol_data[i] = 0.0;
			}
		}
		UpdateOffset(&cgOffset, &gcOffset);
	}
	if (IDA) {
		CALL_SET(IDAWFtolerances, cvEwtSetVV);
		CALL_SET(IDASetNonlinConvCoef, NON_LINEAR_CONV_TEST_COEFF);
		CALL_SET2(IDARootInit, (long)comp->nCross, g_IDA);
		CALL_SET(DSIDAStepRootInit, check_step_event);
		CALL_SET(DSIDASetAccurateDAEEventFn, comp->DAEMode ? SetAccurateDAECrossing : NULL);
	} else 
#endif
	{
		CALL_SET(CVodeWFtolerances, cvEwtSetVV);

		/* specify the safety factor used in the nonlinear convergence test */
		CALL_SET(CVodeSetNonlinConvCoef, NON_LINEAR_CONV_TEST_COEFF);

		/* specify the root function to handle event indicators */
		CALL_SET2(CVodeRootInit, (long)comp->nCross, g);

		/* specify step event detection function */
		CALL_SET(DSCVodeStepRootInit, check_step_event);
	}

	nGroups = (size_t) QJacobianCG_[cgOffset];
	jac_setup(nGroups, IDA, (size_t) cgOffset, (size_t) gcOffset);

	{
#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
		DYNPropagateDidToThread(comp->did);
		/* Check if SuperLU sparse solver is to be used and compute number non-zero elements */
		DYNPropagateDidToThread(comp->did);
		EXCEPTION_TRY
		const int nz = EnableSparse((int)comp->nStates, &(iData->sparseData), cgOffset, gcOffset);
		int nsparse = 0;
		if (nz)	{
			/* Compute Jacobian structure data to be used for sparse Jacobian evaluation */
			int band_width = 0;

			iData->sparseData.nnz = nz;
			DSSetSparseFunctions(&iData->sparseData.functions);

			/* Only A-part is needed here */
			if ((QJacobianDefined_ & 1) == 1) {
				if (InitializeSparseAnalyticData(iData, (int) comp->nStates, nz)) {
					util_logger(comp, comp->instanceName, FMIError, loggMemErr, "Sparse analytic Jacobian initialization failed");
					goto fail;
				}
			} else
				band_width = PrepareSparse(comp, &(iData->sparseData), (int) comp->nStates, nz, cgOffset, gcOffset);
			int nprocs = getNumProcsCvode(&(iData->sparseData), (int) comp->nStates, nz, band_width);
			if (nprocs <= 0) nprocs = 4;

			util_logger(comp, comp->instanceName, FMIOK, loggUndef,
				"Using SuperLU sparse solver on %d core%s for the integrator Jacobian.", nprocs, (nprocs > 1 ? "s" : ""));

			if (iData->sparseData.functions.sun_sparse_matrix && iData->sparseData.functions.sun_linsol_superlumt) {
				iData->A = (*iData->sparseData.functions.sun_sparse_matrix)((int)comp->nStates, (int)comp->nStates, nz, CSC_MAT, iData->sunctx);
				if (util_check_flag((void*)iData->A, "SUNSparseMatrix", 0, comp)) goto fail;
				iData->linearSolver = (*iData->sparseData.functions.sun_linsol_superlumt)(iData->y, iData->A, nprocs, iData->sunctx);
				if (util_check_flag((void*)iData->linearSolver, "SUNLinSol_SuperLUMT", 0, comp)) goto fail;
			} else
				util_logger(comp, comp->instanceName, FMIFatal, loggUndef, "SUNDIALS sparse functions not found.");
			iData->sparseData.nprocs = nprocs;
#else
		if (0){
		
#endif /* FMU_SOURCE_CODE_EXPORT */
		} else {
			iData->A = SUNDenseMatrix((int)comp->nStates, (int)comp->nStates, iData->sunctx);
			if (util_check_flag((void*)iData->A, "SUNDenseMatrix", 0, comp)) goto fail;

			iData->linearSolver = SUNLinSol_Dense(iData->y, iData->A, iData->sunctx);
			if (util_check_flag((void*)iData->linearSolver, "SUNLinSol_Dense", 0, comp)) goto fail;
		}

		/* Set linear solver */
#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			CALL_SET2(IDASetLinearSolver, iData->linearSolver, iData->A);
			/* use own Jacobian function to enable use of grouping */
			CALL_SET(IDASetJacFn, jac_Jacobian_IDA);
		} else 
#endif		
		{
			CALL_SET2(CVodeSetLinearSolver, iData->linearSolver, iData->A);
			/* use own Jacobian function to enable use of grouping */
#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
			CALL_SET(CVodeSetJacFn, nz ? jac_JacobianSparse : jac_Jacobian);
#else
			CALL_SET(CVodeSetJacFn, jac_Jacobian);
#endif
		}

#if !defined(FMU_SOURCE_CODE_EXPORT)
		nsparse = GetAdditionalFlags(23);
		if (nsparse)
			util_logger(comp, comp->instanceName, FMIOK, loggUndef, 
				"Using SuperLU sparse solver for %d system%s of equations.\n", nsparse, (nsparse > 1 ? "s" : ""));
#endif
#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
		EXCEPTION_CATCH_ALL
			return integrationFatal;
		EXCEPTION_END;
#endif
	}

#ifdef INCLUDE_SUNDIALS_IDA
	/* set maximum number of internal steps for a time step */
	if (IDA) {
		CALL_SET(IDASetMaxNumSteps, MAX_STEPS_BEFORE_REINIT);
		CALL_SET(IDASetMaxNonlinIters, MAX_NON_LIN_ITERS);
		CALL_SET(IDASetMaxErrTestFails, MAX_FAILURES);
		CALL_SET2(IDASetErrHandlerFn, err_msg_handler, comp);
	} else 
#endif 
	{
		/* set maximum number of internal steps for a time step */
		CALL_SET(CVodeSetMaxNumSteps, MAX_STEPS_BEFORE_REINIT);

		/* set maximum no. of nonlinear iterations */
		CALL_SET(CVodeSetMaxNonlinIters, MAX_NON_LIN_ITERS);

		/* set maximum number of error test failures permitted in attempting one step */
		CALL_SET(CVodeSetMaxErrTestFails, MAX_FAILURES);

		CALL_SET2(CVodeSetErrHandlerFn, err_msg_handler, comp);
	}

	/* Initialize state logging */
	if (comp->istruct->mPrintEvent & (1 << 15)) {
		if (!comp->storeResult)
			util_logger(comp, comp->instanceName, FMIWarning, loggUndef, "State logging: Please enable result storing to get state names");
#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			CALL_SET(DSIDASetStateErrorMonitorFn, StateErrorMonitorFn);
		} else 
#endif 
		{
			CALL_SET(DSCVodeSetStateErrorMonitorFn, StateErrorMonitorFn);
		}
#if !defined(FMU_SOURCE_CODE_EXPORT)                
		errCountAllocate((int) comp->nStates);
#endif
	}

	iData->tmp1 = N_VNew_Serial((long)comp->nStates, iData->sunctx);
	iData->tmp2 = N_VNew_Serial((long)comp->nStates, iData->sunctx);
	if (IDA)
		iData->tmp3 = N_VNew_Serial((long)comp->nStates, iData->sunctx);
	if (util_check_flag((void *)(iData->tmp1), "N_VNew_Serial", 0, comp) ||
		util_check_flag((void *)(iData->tmp2), "N_VNew_Serial", 0, comp) ||
		(IDA && util_check_flag((void *)(iData->tmp3), "N_VNew_Serial", 0, comp))) {
			goto fail;
	}
	N_VConst(0.0,iData->tmp1);
	N_VConst(0.0,iData->tmp2);
	if (iData->tmp3)
		N_VConst(0.0,iData->tmp3);

	if (comp->useInputSmoother && comp->usePredictorCompensation) {
		if (InputSmoother_Allocate(comp) != FMIOK)
			goto fail;
	}

	iData->cvode_mem = cvode_mem;
	iData->ewt = N_VNew_Serial((long)comp->nStates, iData->sunctx);
	if (util_check_flag((void *)(iData->ewt), "N_VNew_Serial", 0, comp)) goto fail;
	N_VConst(0.0,iData->ewt);

#ifdef FMU_VERBOSITY
	if (nGroups > 0) {
		util_logger(c, comp->instanceName, FMIOK, loggUndef,
			"grouping will be used when computing J (N = %d)", comp->nStates);
	} else {
		util_logger(c, comp->instanceName, FMIOK, loggUndef,
			"grouping will not be used when computing J (N = %d)", comp->nStates);
	}
#endif

	/*  turned out to be faster to set to 0 */
	comp->istruct->mSolverHandleEq = 0;

	comp->steadyStateReached = 0;

	/* initialize statistics */
	iData->stat.fCalls = 0;
	iData->stat.totTime = (float) clock();


	iData->stat.rejSteps = 0;
	iData->stat.rejIntSteps = 0;
	iData->stat.rejFCalls = 0;
	iData->stat.rejJacCalls = 0;

	iData->stat.maxOrder = 0;

	iData->stat.predictorCompensationFCalls = 0;

#ifdef SAVE_STATE_SUNDIALS
	iData->cvode_called = 0;
#endif

	return integrationOK;

fail:
	if(integration_teardown(comp) == integrationFatal)
		return integrationFatal;
	return integrationError;
}

/* ------------------------------------------------------ */
DYMOLA_STATIC IntegrationStatus integration_teardown(FMIComponent c)
{
	Component* comp = (Component*) c;
	IntegrationStatus ret = integrationOK;
	if (comp == NULL || comp->iData == NULL) {
		return ret;
	}

#ifdef LOG_STATISTICS
	/* log statistics */
	if (comp->iData->cvode_mem != NULL) {
		realtype tolsfac = 0;
		int flag = 0;
		IntegrationData* iData = comp->iData;
		Statistics* stat = &(comp->iData->stat);

		stat->totTime = clock() - stat->totTime;
		stat->totTime /= CLOCKS_PER_SEC;

		updateIntegrationStatistics(comp,&comp->iData->stat);
#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			GET_STATISTIC(IDAGetTolScaleFactor, tolsfac);
		} else 
#endif		
		{
			GET_STATISTIC(CVodeGetTolScaleFactor, tolsfac);
		}

		util_logger(comp, comp->instanceName, FMIOK, loggStats,
			"Sundials %s Statistics\n"
			"    Stop time                                : %.2f s\n"
			"    Simulation time                          : %.2f s\n"
			"    Number of external steps                 : %d\n"
			"    Number of internal steps                 : %d\n"
			"    Number of non-linear iterations          : %ld\n"
			"    Number of non-linear convergence failures: %ld\n"
			"    Number of f function evaluations         : %ld\n"
			"    Number of g function evaluations         : %ld\n"
			"    Number of Jacobian-evaluations (direct)  : %ld\n"
			"    Maximum integration order                : %d\n"
			"    Suggested tolerance scale factor         : %.1f\n"
			"    Grouping used                            : %s\n",
			IDA ? "IDA" : "CVode", comp->time, (GetAdditionalFlags(5) ? -1.0 : stat->totTime), stat->steps, stat->intSteps, stat->niters, stat->ncfails,
			stat->fCalls, stat->ngevals, stat->njevals, stat->maxOrder, tolsfac,	nGroups > 0 ? "yes" : "no");
#if (FMI_VERSION >= FMI_2)
		util_logger(comp, comp->instanceName, FMIOK, loggUndef,
			"Rejected count\n"
			"    Number of external steps                 : %d\n"
			"    Number of internal steps                 : %d\n"
			"    Number of f function evaluations         : %d\n"
			"    Number of Jac function evaluations       : %d\n",
			stat->rejSteps, stat->rejIntSteps,
			stat->rejFCalls, stat->rejJacCalls);
#endif
#if (FMI_VERSION >= FMI_2)
		if (comp->usePredictorCompensation && stat->predictorCompensationFCalls) {
			util_logger(comp, comp->instanceName, FMIOK, loggUndef,
				"Predictor compensation count\n"
				"    Number of f function evaluations         : %ld\n",
				stat->predictorCompensationFCalls);
		}
#endif
	}
#ifdef STORE_RESULT
	if (comp->istruct->mPrintEvent & (1 << 15)) {
		const char* dummy = NULL;
		if (!comp->storeResult)
			util_logger(comp, comp->instanceName, FMIWarning, loggUndef, "State logging: Please enable result storing to get state names");
		globalComponent2 = comp;
		statisticsErrCount3(comp->time, 0, comp->tStart, comp->storeResult && comp->result && comp->result->phase.xNames ? comp->result->phase.xNames : &dummy);
		globalComponent2 = NULL;
	}
#endif /* STORE_RESULT */
#endif /* LOG_STATISTICS */
	ret =free_data(comp, comp);
	return ret;
}

#ifdef SAVE_STATE_SUNDIALS
#include "cvode_impl.h"
#endif

/* ------------------------------------------------------ */
DYMOLA_STATIC IntegrationStatus integration_step(Component* comp, FMIReal* tout, FMIReal endStepTime)
{
	IntegrationData* iData = comp->iData;
	void* cvode_mem = iData->cvode_mem;
	InterpolationData* ipData = comp->ipData;
	/* initialize in case tret needs to be reported before set by Sundials */
	realtype tret = comp->time;
	IntegrationStatus iStat = integrationOK;
	int done = 0;
	int current_steps_before_reinit = MAX_STEPS_BEFORE_REINIT;
	int accumulated_steps_before_reinit = MAX_STEPS_BEFORE_REINIT;
	const double mNextTimeEventIn = comp->dstruct->mNextTimeEvent;
	{
		int flag;
#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			CALL_SET(IDASetMaxNumSteps, MAX_STEPS_BEFORE_REINIT);
		} else 
#endif		
		{
			CALL_SET(CVodeSetMaxNumSteps, MAX_STEPS_BEFORE_REINIT); 
		}
	}

	if (comp->ipData->useDerivatives && (comp->useInputSmoother || GetAdditionalFlags(25))) {
#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			IDASetStopTime(cvode_mem, endStepTime);
		} else 
#endif			
		{
			CVodeSetStopTime(cvode_mem, endStepTime);
		}
	}

	assert(comp->nStates > 0);
	assert(*tout > comp->time);

	while (!done) {
		int flag;

		/* store previous outputs for use in fmiGetRealOutputDerivatives */
		if (!util_refresh_cache(comp, iDemandOutput, "fetching current output", NULL)) {
			memcpy(ipData->outputsPrev, comp->outputs, comp->nOut * sizeof(FMIReal));
			ipData->timePrev = comp->time;
		}

#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			flag = IDASolve(cvode_mem, *tout, &tret, iData->y, iData->yDot, IDA_NORMAL);
		} else 
#endif		
		{
			flag = CVode(cvode_mem, *tout, iData->y, &tret, CV_NORMAL);
		}
#ifdef SAVE_STATE_SUNDIALS
		iData->cvode_called = 1;
#endif
		if (comp->istruct)
			comp->istruct->mBlockUnblockSmoothCrossings = 2;
		switch (flag) {

		   case CV_SUCCESS:
		   case CV_TSTOP_RETURN:
			   done = 1;
			   break;

		   case CV_TOO_CLOSE:
			   /* Sundials simply gives up on finding initial step size if the next step after a restart is too small.
			   *  Must explicitly set initial step size as a workaround. */
			   {
				   realtype hcur;

				   flag = CVodeGetCurrentStep(cvode_mem, &hcur);
				   if (util_check_flag(&flag, "CVodeGetCurrentStep", 1, comp)) {
					   iStat = integrationError;
					   goto fail;
				   }
				   if (hcur == 0) {
					   /* next step size not yet computed */ 
					   /* somewhat arbitrary value but should never happen on sane usage */ 
					   hcur = 2 * UNIT_ROUNDOFF * (1 + SUNMAX(SUNRabs(*tout), SUNRabs(comp->time)));
				   }
				   util_logger(comp, comp->instanceName, FMIOK, loggUndef,
					   "Internal step too small at t = %f, setting initial step size explicitly to %.16e", comp->time, hcur);
				   CALL_SET(CVodeSetInitStep, hcur);
			   }
			   break;

		   case CV_TOO_MUCH_WORK:
			   {
				   /* CVode may get stuck with a small step size. It sometimes helps to reinitialize. 
				      The number of internal steps between reinitialization are successively increased. */
				   if (accumulated_steps_before_reinit < MAX_STEPS_TOTAL) {
					   int diff;
					   current_steps_before_reinit *= STEPS_BEFORE_REINIT_FACTOR;
					   accumulated_steps_before_reinit += current_steps_before_reinit;
					   diff = MAX_STEPS_TOTAL - accumulated_steps_before_reinit;
					   if (diff < 0) {
						   current_steps_before_reinit += diff;
						   accumulated_steps_before_reinit += diff;
					   }
					   fmiSetTime_(comp, tret);
			   		   if (iStat = integration_reinit(comp, 1)) {
						   goto fail;
					   }
#ifdef INCLUDE_SUNDIALS_IDA
					   if (IDA) {
						   CALL_SET(IDASetMaxNumSteps, current_steps_before_reinit);
					   } else 
#endif					   
					   {
						   CALL_SET(CVodeSetMaxNumSteps, current_steps_before_reinit);
					   }
				   } else {
					   util_check_flag(&flag, "CVode", 1, comp);
					   iStat = integrationError;
					   goto fail;
				   }
			   }
			   break;

		   case CV_ROOT_RETURN:
			   {
				   /* state or step event occurred */
				   FMIStatus status;
#if (FMI_VERSION >= FMI_2)
				   FMIBoolean terminateSimulation;
#else
				   FMIEventInfo eventInfo;
#endif
				   comp->anyNonEvent = FMIFalse;

				   integration_extrapolate_inputs(comp, tret);
				   fmiSetTime_(comp, tret);
#ifdef LOG_EVENTS
				   if (comp->istruct->mTriggerStepEvent != 0) {
					   util_logger(comp, comp->instanceName, FMIOK, loggUndef, "%.12f: step event", comp->time);
				   } else {
					   util_logger(comp, comp->instanceName, FMIOK, loggUndef,
						   "%.12f: state event", tret);
				   }
#endif
				   INT_RESULT_SAMPLE(FMITrue);
				   status = util_event_update(comp, FMIFalse, 
#if (FMI_VERSION >= FMI_2)					   
					   &terminateSimulation
#else					   
					   &eventInfo
#endif					  
					   );
				   if (status != FMIOK) {
					   iStat = integrationError;
					   goto fail;
				   }
				   INT_RESULT_SAMPLE(FMITrue);

#if (FMI_VERSION >= FMI_2)
				   if (terminateSimulation == FMITrue) {
#else
				   if (eventInfo.terminateSimulation == FMITrue) {
#endif
					   return integrationTerminate;
				   }

				   if (!comp->anyNonEvent) {
					   if (iStat = integration_reinit(comp, 1)) {
						   goto fail;
					   }
				   } else if (comp->DAEMode) {
					   iData->DAEMinorEventRestart = FMITrue;
					   iData->DAEMinorEventRestartTime = tret;
				   }

				   comp->istruct->mBlockUnblockSmoothCrossings = 1;

				   if (comp->dstruct->mNextTimeEvent != mNextTimeEventIn && comp->dstruct->mNextTimeEvent < *tout)
					   /* An event has revealed a new time event before tout, reduce tout */
					   *tout = comp->dstruct->mNextTimeEvent;

				   done = (tret == *tout);
			   }
			   break;

		   default:
				util_check_flag(&flag, "CVode", 1, comp);
				iStat = integrationError;
				goto fail;
		}
	}

	/* check if we are reasonably close to requested stop time */
	if (fabs(tret) - fabs(*tout) > SMALL_TIME_DEV(comp->time)) {
		util_logger(comp, comp->instanceName, FMIError, loggUndef,
			"Internal step error: tret = %.16e  !=  tout = %.16e", tret, *tout);
		iStat = integrationError;
		goto fail;
	}

	iData->stat.steps++;
	return integrationOK;

fail:
	return iStat;
}
#if (FMI_VERSION >= FMI_2)
DYMOLA_STATIC IntegrationStatus integration_step_return_at_event(Component* comp, FMIReal* tout, FMIBoolean handleEvent, FMIBoolean earlyReturn, FMIReal* tRet, FMIBoolean* anyEvent, FMIReal endStepTime)
{
	IntegrationData* iData = comp->iData;
	void* cvode_mem = iData->cvode_mem;
	InterpolationData* ipData = comp->ipData;
	/* initialize in case tret needs to be reported before set by Sundials */
	realtype tret = comp->time;
	IntegrationStatus iStat = integrationOK;
	int done = 0;
	int current_steps_before_reinit = MAX_STEPS_BEFORE_REINIT;
	int accumulated_steps_before_reinit = MAX_STEPS_BEFORE_REINIT;
	const double mNextTimeEventIn = comp->dstruct->mNextTimeEvent;
	{
		int flag;
#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			CALL_SET(IDASetMaxNumSteps, MAX_STEPS_BEFORE_REINIT);
		} else 
#endif		
		{
			CALL_SET(CVodeSetMaxNumSteps, MAX_STEPS_BEFORE_REINIT);
		}
	}

	if (comp->ipData->useDerivatives && (comp->useInputSmoother || GetAdditionalFlags(25))) {
#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			IDASetStopTime(cvode_mem, endStepTime);
		} else 
#endif			
		{
			CVodeSetStopTime(cvode_mem, endStepTime);
		}
	}


	assert(comp->nStates > 0);
	assert(*tout > comp->time);

	while (!done) {
		int flag;

		/* store previous outputs for use in fmiGetRealOutputDerivatives */
		if (!util_refresh_cache(comp, iDemandOutput, "fetching current output", NULL)) {
			memcpy(ipData->outputsPrev, comp->outputs, comp->nOut * sizeof(FMIReal));
			ipData->timePrev = comp->time;
		}

#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			flag = IDASolve(cvode_mem, *tout, &tret, iData->y, iData->yDot, IDA_NORMAL);
		} else 
#endif		
		{
			flag = CVode(cvode_mem, *tout, iData->y, &tret, CV_NORMAL);
		}
#ifdef SAVE_STATE_SUNDIALS
		iData->cvode_called = 1;
#endif
		if (comp->istruct)
			comp->istruct->mBlockUnblockSmoothCrossings = 2;
		switch (flag) {

		   case CV_SUCCESS:
		   case CV_TSTOP_RETURN:
			   done = 1;
			   break;

		   case CV_TOO_CLOSE:
			   /* Sundials simply gives up on finding initial step size if the next step after a restart is too small.
			   *  Must explicitly set initial step size as a workaround. */
			   {
				   realtype hcur;

				   flag = CVodeGetCurrentStep(cvode_mem, &hcur);
				   if (util_check_flag(&flag, "CVodeGetCurrentStep", 1, comp)) {
					   iStat = integrationError;
					   goto fail;
				   }
				   if (hcur == 0) {
					   /* next step size not yet computed */ 
					   /* somewhat arbitrary value but should never happen on sane usage */ 
					   hcur = 2 * UNIT_ROUNDOFF * (1 + SUNMAX(SUNRabs(*tout), SUNRabs(comp->time)));
				   }
				   util_logger(comp, comp->instanceName, FMIOK, loggUndef,
					   "Internal step too small at t = %f, setting initial step size explicitly to %.16e", comp->time, hcur);
				   CALL_SET(CVodeSetInitStep, hcur);
			   }
			   break;

		   case CV_TOO_MUCH_WORK:
			   {
				   /* CVode may get stuck with a small step size. It sometimes helps to reinitialize.
					  The number of internal steps between reinitialization are successively increased. */
				   if (accumulated_steps_before_reinit < MAX_STEPS_TOTAL) {
					   int diff;
					   current_steps_before_reinit *= STEPS_BEFORE_REINIT_FACTOR;
					   accumulated_steps_before_reinit += current_steps_before_reinit;
					   diff = MAX_STEPS_TOTAL - accumulated_steps_before_reinit;
					   if (diff < 0) {
						   current_steps_before_reinit += diff;
						   accumulated_steps_before_reinit += diff;
					   }
					   fmiSetTime_(comp, tret);
			   		   if (iStat = integration_reinit(comp, 1)) {
						   goto fail;
					   }
#ifdef INCLUDE_SUNDIALS_IDA
					   if (IDA) {
						   CALL_SET(IDASetMaxNumSteps, current_steps_before_reinit);
					   } else 
#endif					   
					   {
						   CALL_SET(CVodeSetMaxNumSteps, current_steps_before_reinit);
					   }
				   } else {
					   util_check_flag(&flag, "CVode", 1, comp);
					   iStat = integrationError;
					   goto fail;
				   }
			   }
			   break;

		   case CV_ROOT_RETURN:
		   {
			   /* state or step event occurred */

			   integration_extrapolate_inputs(comp, tret);
			   fmiSetTime_(comp, tret);
			   *tRet = (FMIReal)tret;
			   *anyEvent = FMITrue;
#ifdef LOG_EVENTS
			   if (comp->istruct->mTriggerStepEvent != 0) {
				   util_logger(comp, comp->instanceName, FMIOK, loggUndef, "%.12f: step event", comp->time);
			   } else {
				   util_logger(comp, comp->instanceName, FMIOK, loggUndef,
					   "%.12f: state event", tret);
			   }
#endif
			   INT_RESULT_SAMPLE(FMITrue);

			   if (handleEvent) {
				   FMIStatus status;
				   FMIBoolean terminateSimulation;
				   /* Handle the event as in integrator_step */
				   status = util_event_update(comp, FMIFalse,
					   &terminateSimulation
				   );
				   if (status != FMIOK) {
					   iStat = integrationError;
					   goto fail;
				   }
				   INT_RESULT_SAMPLE(FMITrue);

				   if (terminateSimulation == FMITrue) {
					   return integrationTerminate;
				   }

				   if (!comp->anyNonEvent) {
					   if (iStat = integration_reinit(comp, 1)) {
						   goto fail;
					   }
				   } else if (comp->DAEMode) {
					   iData->DAEMinorEventRestart = FMITrue;
					   iData->DAEMinorEventRestartTime = tret;
				   }

			   }
			   comp->istruct->mBlockUnblockSmoothCrossings = 1;
			   if (earlyReturn) {
				   /* Return at event point */
				   
				   return integrationEvent;
			   } else {
				   if (comp->dstruct->mNextTimeEvent != mNextTimeEventIn && comp->dstruct->mNextTimeEvent < *tout)
					   /* An event has revealed a new time event before tout, reduce tout */
					   *tout = comp->dstruct->mNextTimeEvent;
				   break;
			   }
			   done = (tret == *tout);
		   }

		   default:
				util_check_flag(&flag, "CVode", 1, comp);
				iStat = integrationError;
				goto fail;
		}
	}

	/* check if we are reasonably close to requested stop time */
	if (fabs(tret) - fabs(*tout) > SMALL_TIME_DEV(comp->time)) {
		util_logger(comp, comp->instanceName, FMIError, loggUndef,
			"Internal step error: tret = %.16e  !=  tout = %.16e", tret, *tout);
		goto fail;
	}
	*tRet = (FMIReal) tret;
	iData->stat.steps++;
	return integrationOK;

fail:
	*tRet = (FMIReal) tret;
	return integrationError;
}
#endif


DYMOLA_STATIC void updateIntegrationStatistics(Component* comp, Statistics* stat){

	long nniters, nncfails, ngevals, njevals, nfevals, nsteps;
	int flag;
#ifdef INCLUDE_SUNDIALS_IDA
	if (IDA) {
		GET_STATISTIC(IDAGetNumResEvals, nfevals);
		stat->fCalls += nfevals;
		GET_STATISTIC(IDAGetNumNonlinSolvIters, nniters);
		stat->niters += nniters;
		GET_STATISTIC(IDAGetNumNonlinSolvConvFails, nncfails);
		stat->ncfails += nncfails;
		GET_STATISTIC(IDAGetNumGEvals, ngevals);
		stat->ngevals += ngevals;
		GET_STATISTIC(IDAGetNumJacEvals, njevals);
		stat->njevals += njevals;
		GET_STATISTIC(IDAGetNumSteps, nsteps);
		stat->intSteps += nsteps;
	} else 
#endif
	{
		GET_STATISTIC(CVodeGetNumRhsEvals, nfevals);
		stat->fCalls += nfevals;
		GET_STATISTIC(CVodeGetNumNonlinSolvIters, nniters);
		stat->niters += nniters;
		GET_STATISTIC(CVodeGetNumNonlinSolvConvFails, nncfails);
		stat->ncfails += nncfails;
		GET_STATISTIC(CVodeGetNumGEvals, ngevals);
		stat->ngevals += ngevals;
		GET_STATISTIC(CVodeGetNumJacEvals, njevals);	
		stat->njevals += njevals;
		GET_STATISTIC(CVodeGetNumSteps, nsteps);
		stat->intSteps += nsteps;
	}
}

/* ------------------------------------------------------ */
DYMOLA_STATIC IntegrationStatus integration_reinit(Component* comp, int updateStatistics)
{
	int flag;

	assert(comp->nStates > 0);
  
	if (updateStatistics) {
		updateIntegrationStatistics(comp, &comp->iData->stat);
	}

#ifdef INCLUDE_SUNDIALS_IDA
	if (IDA) {
		flag = IDAReInit(comp->iData->cvode_mem, comp->time, comp->iData->y, comp->iData->yDot);
	} else 
#endif	
	{
		flag = CVodeReInit(comp->iData->cvode_mem, comp->time, comp->iData->y);
		CVodeSetInitStep(comp->iData->cvode_mem, 0.0);
	}

#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
	DYNPropagateDidToThread(comp->did);
	EXCEPTION_TRY
	if (comp->iData->sparseData.nnz && comp->iData->sparseData.functions.ds_superlumt_reinit)
		(*comp->iData->sparseData.functions.ds_superlumt_reinit)(comp->iData->linearSolver);
	EXCEPTION_CATCH_ALL
		return integrationFatal;
	EXCEPTION_END;

#endif

	if (util_check_flag(&flag, "CVodeReInit", 1, comp)) return integrationError;
	comp->reinitCvode = 0;
	return integrationOK;
}

/* ------------------------------------------------------ */
DYMOLA_STATIC void integration_sync_extrapolation_input(Component* comp, size_t ix)
{
	if (comp->ipData != NULL && comp->ipData->inputsT0 != NULL) {
		comp->ipData->inputsT0[ix] = comp->inputs[ix];
	}
}

/* ------------------------------------------------------ */
DYMOLA_STATIC void integration_extrapolate_inputs(Component* comp, double t)
{
#if (FMI_VERSION >= FMI_3)
	if (comp->intermediateUpdate && comp->nRequiredIntermediateInputs>0) {
		fmi3Boolean earlyReturnRequested = FMIFalse;
		fmi3Float64 earlyReturnTime = FMIFalse;
		/* Allowed to set inputs - not to get variables, and step not completed and early return not allowed */
		(*(comp->intermediateUpdate))(comp->ie, t, FMITrue, FMIFalse, FMIFalse, FMIFalse, &earlyReturnRequested, &earlyReturnTime);
	} else
#endif
	if (comp->ipData->useDerivatives) {
		size_t i;
		double dt = (t - comp->ipData->derivativeTime);
		for (i = 0; i < comp->nIn; i++) {
			comp->inputs[i] = comp->ipData->inputsT0[i] + comp->ipData->inputDerivatives[i] * dt;
		}
	}
}

#if (FMI_VERSION >= FMI_2)
/* ------------------------------------------------------ */
DYMOLA_STATIC FMIStatus integration_get_state(Component* source, Component* target)
{
	if (source->nStates > 0) {
		if (source->iData != NULL) {
			/* possible to reuse iData allocation from target since no sizes of sub parts can have changed */
			Statistics stat = source->iData->stat;
			if (clone_data(target, source) != integrationOK) {
				goto mem_fail;
			}
			if (clone_ipData(target, source) != FMIOK)
				goto mem_fail;
			updateIntegrationStatistics(source,&stat);
			target->iData->stat = stat;
		} else if (target->iData != NULL) {
			/* free old iData to avoid memory leak */
			if(free_data(source, target) == integrationFatal)
				return FMIFatal;
		}
	}
	
	return FMIOK;

mem_fail:
	util_logger(source, source->instanceName, FMIError, loggMemErr, "memory allocation failed");
	if(free_data(source, target)== integrationFatal)
		return FMIFatal;
	return util_error(source);
}

/* ------------------------------------------------------ */
DYMOLA_STATIC FMIStatus integration_set_state(Component* source, Component* target, FMIBoolean reinit_integration)
{
	if (target->nStates == 0) {
		// may occur when saving FMU states into uninitialized components
		// as in integration_setup, use dummy state to simplify logic
		target->nStates = 1;
	}

	if (source->iData != NULL) {
		
		if (clone_data(target, source) != integrationOK) {
			goto mem_fail;
		}
		if (clone_ipData(target, source) != FMIOK){
				goto mem_fail;
		}

		if (reinit_integration) {
			/* count of rejected steps before copying statistics */
			source->iData->stat.rejSteps += target->iData->stat.steps - source->iData->stat.steps;
			source->iData->stat.rejIntSteps += target->iData->stat.intSteps - source->iData->stat.intSteps;
			source->iData->stat.rejFCalls += target->iData->stat.fCalls - source->iData->stat.fCalls;

			memcpy(&target->iData->stat, &source->iData->stat, sizeof(Statistics));
		}

#ifdef SAVE_STATE_SUNDIALS
		integration_reinit(target, 1);
		/* code fetched from CVAckpntGet and modified */
		{
			CVodeMem cv_mem = (CVodeMem)target->iData->cvode_mem;
			CVodeCheckPointData* ck_mem = &target->iData->ck_mem;
			N_Vector* cv_zn = cv_mem->cv_zn;
			N_Vector* ck_zn = ck_mem->ck_zn;
			int q = cv_mem->cv_q;
			int qmax = cv_mem->cv_qmax;
			int j;

			/* Copy parameters from check point data structure */
			cv_mem->cv_nst = ck_mem->ck_nst;
			cv_mem->cv_tretlast = ck_mem->ck_tretlast;
			cv_mem->cv_q = ck_mem->ck_q;
			cv_mem->cv_qprime = ck_mem->ck_qprime;
			cv_mem->cv_qwait = ck_mem->ck_qwait;
			cv_mem->cv_L = ck_mem->ck_L;
			cv_mem->cv_gammap = ck_mem->ck_gammap;
			cv_mem->cv_h = ck_mem->ck_h;
			cv_mem->cv_hprime = ck_mem->ck_hprime;
			cv_mem->cv_hscale = ck_mem->ck_hscale;
			cv_mem->cv_eta = ck_mem->ck_eta;
			cv_mem->cv_etamax = ck_mem->ck_etamax;
			cv_mem->cv_saved_tq5 = ck_mem->ck_saved_tq5;

			/* Copy the arrays from check point data structure */
			for (j = 0; j <= q; j++) N_VScale(1.0, ck_zn[j], cv_zn[j]);
			if (q < qmax) N_VScale(1.0, ck_zn[qmax], cv_zn[qmax]);

			for (j = 0; j <= L_MAX; j++)     cv_mem->cv_tau[j] = ck_mem->ck_tau[j];
			for (j = 0; j <= NUM_TESTS; j++) cv_mem->cv_tq[j] = ck_mem->ck_tq[j];
			for (j = 0; j <= q; j++)         cv_mem->cv_l[j] = ck_mem->ck_l[j];
		}
#endif
	}
	else if (target->iData == NULL) {
		/* need to setup integration data from scratch */
		IntegrationStatus ret = integrationOK;
		switch(integration_setup(target, source->relativeTolerance)){
		case integrationOK:
			break;
		case integrationError:
			return util_error(target);
		case integrationFatal:
		default:
			return FMIFatal;
		}
	}

/* for explicit saving of Sundials states, we already re-initialized _before_ restoring */
#ifndef SAVE_STATE_SUNDIALS
	if (reinit_integration)
		integration_reinit(target,0);
#endif
	return FMIOK;

mem_fail:
	util_logger(target, target->instanceName, FMIError, loggMemErr, "memory allocation failed");
	return util_error(source);
}

/* ------------------------------------------------------ */
DYMOLA_STATIC IntegrationStatus integration_free_state(Component* comp, Component* target)
{
	IntegrationStatus ret = integrationOK;
	if (target->nStates > 0) {
#ifdef SAVE_STATE_SUNDIALS
		/* free cloned Sundials internal data */
		CVodeCheckPointData* ck_mem = &target->iData->ck_mem;
		N_Vector* ck_zn = ck_mem->ck_zn;

		int q = ck_mem->ck_q;
		int j;

		for (j = 0; j < q; j++) {
			N_VDestroy(ck_zn[j]);
		}

		if (q < ck_mem->ck_qmax) {
			N_VDestroy(ck_zn[ck_mem->ck_qmax]);
		}
#endif
		ret = free_data(comp, target);
	}
	integration_free_ipData(target);
	return ret;
}

#endif /* FMI_2 */


/* ----------------- local function definitions ----------------- */


static FMIStatus clone_ipData(Component* target, Component* source)
{

	InterpolationData* ipData = target->ipData;

	if(ipData == NULL){
		FMIStatus s = integration_allocate_ipData(target);
		if(s == FMIError || s == FMIFatal)
			goto mem_fail;
		ipData = target->ipData;
	}
	/* handle input derivative data */
	ipData->useDerivatives = source->ipData->useDerivatives;
	if (ipData->useDerivatives) {
		size_t nu = source->nIn;
		assert(source->ipData->inputDerivatives != NULL);
		assert(nu > 0);
		memcpy(ipData->inputDerivatives, source->ipData->inputDerivatives, nu * sizeof(FMIReal));
		memcpy(ipData->inputsT0, source->ipData->inputsT0, nu * sizeof(FMIReal));
		ipData->derivativeTime = source->ipData->derivativeTime;
		if (ipData->inputUsesSmoother && source->ipData->inputUsesSmoother)
			memcpy(ipData->inputUsesSmoother, source->ipData->inputUsesSmoother, nu * sizeof(FMIBoolean));
		if (ipData->inputForSmoother && source->ipData->inputForSmoother)
			memcpy(ipData->inputForSmoother, source->ipData->inputForSmoother, nu * sizeof(FMIReal));
	}

	/* handle ouput derivative data */
	{
		size_t ny = source->nOut;
		assert(ipData->outputsPrev != NULL);
		memcpy(ipData->outputsPrev, source->ipData->outputsPrev, ny * sizeof(FMIReal));
		ipData->timePrev = source->ipData->timePrev;
	}
	return FMIOK;
mem_fail:
	util_logger(source, source->instanceName, FMIError, loggMemErr, "memory allocation failed");
	integration_free_ipData(target);
	return FMIError;
}

static IntegrationStatus clone_data(Component* target, Component* source)
{
	IntegrationData* iData;
	
	assert(target->nStates > 0);
	assert(source->nStates > 0);
	assert(source->iData != NULL);

	iData = target->iData;
	if (iData == NULL) {
		IntegrationStatus ret;
		ret = integration_setup(target, 1e-4);
		if(ret != integrationOK)
			return ret;
		iData = target->iData;
	}

	/* don't use N_V_COPY since it also copies ops member */
	N_VScale(1.0, source->iData->abstol, iData->abstol);
	N_VScale(1.0, source->iData->reltol, iData->reltol);
	N_VScale(1.0, source->iData->ewt, iData->ewt);
	N_VScale(1.0, source->iData->tmp1, iData->tmp1);
	N_VScale(1.0, source->iData->tmp2, iData->tmp2);
	if (source->iData->tmp3)
		N_VScale(1.0, source->iData->tmp3, iData->tmp3);
	
	CopySparseFixData(source->nStates, &iData->sparseData, &source->iData->sparseData);
#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
	if (iData->sparseAnalyticData && source->iData->sparseAnalyticData)
		CopySparseAnalyticData(iData, source->iData);
#endif

#ifndef FMU_SOURCE_CODE_EXPORT
	if (source->iData->steadyStateUtils) {
		extern struct SteadyStateUtils* CopySteadyStateUtils(struct SteadyStateUtils* utils);
		iData->steadyStateUtils = CopySteadyStateUtils(source->iData->steadyStateUtils);
	}
#endif

	if (source->iData->f_vrefs && iData->f_vrefs)
		memcpy(iData->f_vrefs, source->iData->f_vrefs, source->nStates * sizeof(FMIValueReference));
	if (source->iData->u_vrefs && iData->u_vrefs)
		memcpy(iData->u_vrefs, source->iData->u_vrefs, source->nIn * sizeof(FMIValueReference));
	/* No need to copy the work arrays delta_udot and df_du_delta_udot. Should not copy save_state. */

#ifdef SAVE_STATE_SUNDIALS
	/* code fetched from CVAckpntNew and modified */
	if (iData->cvode_called)
	{
		CVodeMem cv_mem = (CVodeMem)iData->cvode_mem;
		CVodeCheckPointData* ck_mem = &iData->ck_mem;
		N_Vector* cv_zn = cv_mem->cv_zn;
		N_Vector* ck_zn = ck_mem->ck_zn;
		int q = cv_mem->cv_q;
		int qmax = cv_mem->cv_qmax;
		int j;

		if (!target->allocDone) {
			/* Test if we need to allocate space for the last zn.
			* NOTE: zn(qmax) may be needed for a hot restart, if an order
			* increase is deemed necessary at the first step after a check point */
			for (j=0; j<=q; j++) {
				ck_zn[j] = N_VClone(cv_mem->cv_tempv);
				if (ck_zn[j] == NULL) {
					int jj;
					for (jj=0; jj<j; jj++) N_VDestroy(ck_zn[jj]);
					goto mem_fail;
				}
			}

			if (q < qmax) {
				ck_zn[qmax] = N_VClone(cv_mem->cv_tempv);
				if (ck_zn[qmax] == NULL) {
					int jj;
					for (jj=0; jj<=q; jj++) N_VDestroy(ck_zn[jj]);
					goto mem_fail;
				}
			}
		}

		/* Load check point data from cv_mem */

		for (j=0; j<=q; j++) N_VScale(1.0, cv_zn[j], ck_zn[j]);
		if ( q < qmax ) N_VScale(1.0, cv_zn[qmax], ck_zn[qmax]);

		/* TODO: try memcpy */
		for (j=0; j<=L_MAX; j++)     ck_mem->ck_tau[j] = cv_mem->cv_tau[j];
		for (j=0; j<=NUM_TESTS; j++) ck_mem->ck_tq[j] = cv_mem->cv_tq[j];
		for (j=0; j<=q; j++)         ck_mem->ck_l[j] = cv_mem->cv_l[j];

		ck_mem->ck_nst       = cv_mem->cv_nst;
		ck_mem->ck_tretlast  = cv_mem->cv_tretlast;
		ck_mem->ck_q         = cv_mem->cv_q;
		ck_mem->ck_qmax         = cv_mem->cv_qmax;
		ck_mem->ck_qprime    = cv_mem->cv_qprime;
		ck_mem->ck_qwait     = cv_mem->cv_qwait;
		ck_mem->ck_L         = cv_mem->cv_L;
		ck_mem->ck_gammap    = cv_mem->cv_gammap;
		ck_mem->ck_h         = cv_mem->cv_h;
		ck_mem->ck_hprime    = cv_mem->cv_hprime;
		ck_mem->ck_hscale    = cv_mem->cv_hscale;
		ck_mem->ck_eta       = cv_mem->cv_eta;
		ck_mem->ck_etamax    = cv_mem->cv_etamax;
		ck_mem->ck_saved_tq5 = cv_mem->cv_saved_tq5;
	}
	/* End of: code fetched from CVAckpntNew and modified */
#endif

	return integrationOK;
#ifdef SAVE_STATE_SUNDIALS
mem_fail:
	util_logger(source, source->instanceName, FMIError, loggMemErr, "memory allocation failed");
	FREE_MEMORY(target->iData);
#ifndef FMU_SOURCE_CODE_EXPORT
	if (iData->steadyStateUtils) {
		extern void FreeSteadyStateUtils(struct SteadyStateUtils* utils);
		FreeSteadyStateUtils(iData->steadyStateUtils);
		iData->steadyStateUtils = NULL;
	}
#endif
	return integrationError;
#endif
}

DYMOLA_STATIC FMIStatus integration_allocate_ipData(Component* comp){

	if(!comp)
		return FMIFatal;
	if(!comp->ipData){

		comp->ipData = (InterpolationData*) ALLOCATE_MEMORY(1, sizeof(InterpolationData));
		if( comp->ipData == NULL) {
			goto fail;
		}

		comp->ipData->inputDerivatives = (FMIReal*) ALLOCATE_MEMORY(comp->nIn + 1, sizeof(FMIReal));
		comp->ipData->inputsT0 = (FMIReal*) ALLOCATE_MEMORY(comp->nIn + 1, sizeof(FMIReal));
		comp->ipData->outputsPrev = (FMIReal*) ALLOCATE_MEMORY(comp->nOut + 1, sizeof(FMIReal));

		if (comp->ipData->inputDerivatives == NULL || comp->ipData->inputsT0 == NULL || comp->ipData->outputsPrev == NULL) {
				goto fail;
		}

		comp->ipData->useDerivatives = 0;

		if (comp->useInputSmoother) {
			size_t i;
			comp->ipData->inputUsesSmoother = (FMIBoolean*) ALLOCATE_MEMORY(comp->nIn + 1, sizeof(FMIBoolean));
			comp->ipData->inputForSmoother = (FMIReal*) ALLOCATE_MEMORY(comp->nIn + 1, sizeof(FMIReal));
			if (comp->ipData->inputUsesSmoother == NULL || comp->ipData->inputForSmoother == NULL)
				goto fail;
			for (i = 0; i < comp->nIn; ++i) {
				comp->ipData->inputDerivatives[i] = 0.0;
				comp->ipData->inputsT0[i] = comp->inputs[i];
				comp->ipData->inputUsesSmoother[i] = FMIFalse;
				comp->ipData->inputForSmoother[i] = comp->inputs[i];
			}
		}
	}
	return FMIOK;
fail:
	util_logger(comp, comp->instanceName, FMIFatal, loggMemErr, "memory allocation failed\n");
	return FMIFatal;
}

DYMOLA_STATIC void integration_free_ipData(Component* comp){
	if(comp && comp->ipData){
		InterpolationData* ipData = comp->ipData;
		FREE_MEMORY(ipData->outputsPrev);
		FREE_MEMORY(ipData->inputDerivatives);
		FREE_MEMORY(ipData->inputsT0);
		if (ipData->inputUsesSmoother) FREE_MEMORY(ipData->inputUsesSmoother);
		if (ipData->inputForSmoother) FREE_MEMORY(ipData->inputForSmoother);
		FREE_MEMORY(comp->ipData);
	}
}

/* ------------------------------------------------------ */

/* --- Input smoother and predictor compensation --------
 * The input smoother delays the current input to the end of the current doStep interval. The input is interpolated from the previous input to
 * the current input over the interval. In this way the input is smoothened to a continuous, piecewise linear signal. Due to the improved
 * smootheness, there is less demand for re-initializing the integrator. However, the discontinuous input derivatives still cause problems for
 * the integrator when the order is two or larger. As an optional feature a predictor correction may be applied to compensate for the jump in
 * input derivatives. This improves the integrator predictor resulting in better guesses for the Newton iteration as well as an improved error
 * estimate, usually leading to fewer f-evaluations, Jacobian computations, and rejected steps.
 */

/* Check that the input smoother is supported under the current export settings. */
DYMOLA_STATIC FMIStatus InputSmoother_CheckSettings(Component* comp)
{
	if (comp->useInputSmoother) {
#if (FMI_VERSION < FMI_2)
		util_logger(comp, comp->instanceName, FMIFatal, loggBadCall, "Input smoother can only be used with FMI 2 and FMI 3");
		return FMIFatal;
#endif
		if (comp->istruct->mInlineIntegration) {
			util_logger(comp, comp->instanceName, FMIFatal, loggBadCall, "Input smoother cannot be used with inline integration");
			return FMIFatal;
		}
		if (comp->useInputSmoother > 2) {
			util_logger(comp, comp->instanceName, FMIFatal, loggBadCall, "The mutually exclusive options \"input smoother\" and \"zero-order hold without reinit\" are both enabled. Please disable one of the options");
			return FMIFatal;
		} else if (comp->useInputSmoother == 1 && comp->usePredictorCompensation) {
			util_logger(comp, comp->instanceName, FMIFatal, loggBadCall, "Predictor compensation cannot be used with the option \"zero-order hold without reinit\"");
			return FMIFatal;
		}
		if (comp->usePredictorCompensation && comp->DAEMode) {
			util_logger(comp, comp->instanceName, FMIFatal, loggBadCall, "Predictor compensation cannot be used with DAE mode");
			return FMIFatal;
		}
		if (comp->nIn == 0 || comp->nStates == 0) {
			/* In this case it is okay to disable the input smoother. In other cases it may happen that the changes to preCont has an effect on the simulation */
			comp->useInputSmoother = 0;
			comp->usePredictorCompensation = 0;
			util_logger(comp, comp->instanceName, FMIWarning, loggUndef, "The FMU has no %s, disabling input smoother.", comp->nIn == 0 ? "inputs" : "states");
		}
		if (comp->useInputSmoother == 2)
			util_logger(comp, comp->instanceName, FMIOK, loggUndef, "Using input smoothing%s.", comp->usePredictorCompensation ? " with predictor compensation" : "");
		else if (comp->useInputSmoother == 1)
			util_logger(comp, comp->instanceName, FMIOK, loggUndef, "Using zero-order hold without re-initializing the integrator.");
	}
	return FMIOK;
}

/* Allocate memory required for predictor compensation. */
DYMOLA_STATIC FMIStatus InputSmoother_Allocate(Component* comp)
{
	IntegrationData* iData;

	if (!comp || !comp->iData || !comp->useInputSmoother || !comp->usePredictorCompensation)
		return FMIOK;
	
	iData = comp->iData;
	iData->delta_udot = N_VNew_Serial((long) comp->nIn, iData->sunctx);
	iData->df_du_delta_udot = N_VNew_Serial((long) comp->nStates, iData->sunctx);
	iData->f_vrefs = (FMIValueReference*) ALLOCATE_MEMORY(comp->nStates, sizeof(FMIValueReference));
	iData->u_vrefs = (FMIValueReference*) ALLOCATE_MEMORY(comp->nIn, sizeof(FMIValueReference));
	if (!iData->delta_udot || !iData->df_du_delta_udot || !iData->f_vrefs || !iData->u_vrefs)
		return FMIFatal;

	return FMIOK;
}

/* Initialize the input smoother. Determine which inputs should use the input smoother. Build variable reference arrays. */
DYMOLA_STATIC FMIStatus InputSmoother_Initialize(Component* comp)
{
	size_t i;
	FMIInteger* basetype = NULL;
	struct DeclarePhase phase = { 0 };

	if (!comp || !comp->useInputSmoother || !comp->inputs || !comp->iData || !comp->ipData || !comp->ipData->inputsT0 
			|| !comp->ipData->inputUsesSmoother || !comp->ipData->inputForSmoother)
		return FMIError;

	/* Use the result of the initialization to initialize the array of previous input values. Note, that the inputs may be set again before 
	   performing the first doStep. Then we ramp up from the result of the initialization to the new inputs. */
	for (i = 0; i < comp->nIn; ++i) {
		comp->ipData->inputsT0[i] = comp->inputs[i];
		comp->ipData->inputForSmoother[i] = comp->inputs[i];
	}

	/* Inquire the model for the continuity of inputs. */
	basetype = (FMIInteger*) ALLOCATE_MEMORY(comp->nIn, sizeof(FMIInteger));
	if (!basetype)
		return FMIError;
	phase.temp.declarePhase_ = 22;
	phase.basetype = basetype;

	declareNew2_((double*) 0, (double*) 0, (double*) 0, (double*) 0, (double*) 0, (double*) 0, (double*) 0, (void**) 0, (int*) 0, 0, &phase);

	/* Discrete inputs (Boolean and Integer) and discrete-time Real inputs should trigger events as normal. Only continuous-time Real inputs 
	   should use the input smoother. */
	for (i = 0; i < comp->nIn; ++i) {
		const FMIInteger currentBaseType = basetype[i];
		if (currentBaseType & dsDiscrete || currentBaseType & dsBoolean || currentBaseType & dsInteger)
			comp->ipData->inputUsesSmoother[i] = FMIFalse;
		else
			comp->ipData->inputUsesSmoother[i] = FMITrue;
	}

	if (basetype)
		FREE_MEMORY(basetype);
	phase.basetype = basetype = NULL;

	/* Initialize predictor compensation. */
	if (comp->usePredictorCompensation && comp->iData->f_vrefs && comp->iData->u_vrefs) {
		IntegrationData* iData = comp->iData;
		FMIValueReference base;
		size_t smooth_index = 0;

		/* Construct value references for directional derivatives. */
		base = (fmiDerivative << 24) + (2 << 29);
		for (i = 0; i < comp->nStates; ++i)
			iData->f_vrefs[i] = base + (FMIValueReference) i;

		base = (fmiInput << 24) + (1 << 29);
		for (i = 0; i < comp->nIn; ++i) {
			if (comp->ipData->inputUsesSmoother[i]) {
				iData->u_vrefs[smooth_index] = base + (FMIValueReference) i;
				++smooth_index;
			}
		}
	}

	return FMIOK;
}

/* Inquire if an input should use the input smoother and in that case store it for setting later, at the beginning of the doStep.
   Discrete inputs (Boolean and Integer) and discrete-time Real inputs should trigger events as normal. Only continuous-time Real inputs
   should use the input smoother. During initialization the input smoother should be disabled. During directional derivative
   computation we need to set the input immediately to allow for perturbations of inputs. */
DYMOLA_STATIC FMIBoolean InputSmoother_SetInput(Component* comp, size_t i, FMIReal value)
{
	if (!comp || !comp->ipData
			|| comp->mStatus == modelInstantiated || comp->mStatus == modelInitializationMode
			|| !comp->ipData->inputUsesSmoother[i])
		return FMIFalse;

	if (comp->istruct && comp->istruct->mInJacobian)
		comp->inputs[i] = value;
	else
		comp->ipData->inputForSmoother[i] = value;
	
	return FMITrue;
}

/* Apply input smoother and potentially modify the integrator predictor to compensate for the jump in the derivative of the input. */
DYMOLA_STATIC FMIStatus InputSmoother_Apply(Component* comp, FMIReal currentCommunicationPoint, FMIReal endStepTime)
{
	size_t i;
	IntegrationData* iData;
	InterpolationData* ipData;
	int q = 0;
	FMIBoolean performPredictorCompensation;
	const FMIReal communicationStepSize = endStepTime - currentCommunicationPoint;

	if (!comp || !comp->useInputSmoother || !comp->inputs || !comp->iData || !comp->ipData || !comp->ipData->inputsT0
			|| !comp->ipData->inputDerivatives || !comp->ipData->inputUsesSmoother)
		return FMIError;

	if (communicationStepSize <= 0.0) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,
			"A positive communication step-size is required when using input smoothing.");
		return util_error(comp);
	}
	if (fabs(comp->time - currentCommunicationPoint) > 2 * SMALL_TIME_DEV(comp->time)) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,
			"currentCommunicationPoint = %g, expected last stop time at %g,\n this indicates a mismatch in the master. This is not allowed when using input smoothing.", currentCommunicationPoint, comp->time);
		return util_error(comp);
	}

	iData = comp->iData;
	ipData = comp->ipData;

	/* Input smoother: Part 1. */

	/* The inputs that aren't smoothed should be kept constant. */
	for (i = 0; i < comp->nIn; ++i) {
		if (comp->ipData->inputUsesSmoother[i])
			comp->inputs[i] = ipData->inputsT0[i]; /* inputs[i] shouldn't have changed for smoothed inputs, but reset it here anyway to make sure. */
		else {
			ipData->inputDerivatives[i] = 0.0;
			ipData->inputsT0[i] = comp->inputs[i];
		}
	}

	/* Predictor compensation: Modify the integrator predictor to compensate for the jump in udot. */
	
	/* The predictor compensation is only applicable when the used order is 2 or larger. It is also unnecessary to apply the compensation if 
	   the integrator anyway will be re-initialized before continuing integration. */
	performPredictorCompensation = (comp->usePredictorCompensation && !comp->reinitCvode && !comp->valWasSet
		&& iData->delta_udot && iData->df_du_delta_udot && iData->f_vrefs && iData->u_vrefs);
	if (performPredictorCompensation) {
#if INCLUDE_SUNDIALS_IDA
		if (IDA)
			/* IDA already has the divided differences of order 2, even if the last step was with order 1, so predictor compensation is always
			   useful as long as the next step is with order 2 or higher. */
			performPredictorCompensation = (IDA_SUCCESS == IDAGetCurrentOrder(iData->cvode_mem, &q) && q >= 2);
		else
#endif
			/* For CVode the xbis part of the Nordsieck array will be zerod if the last order was 1, so no need to perform predictor
			   compensation in that case. Rather predictor compensation is only useful if both the last and the next steps are of order 2 or
			   higher. */
			performPredictorCompensation = (CV_SUCCESS == CVodeGetLastOrder(iData->cvode_mem, &q) && q >= 2
				&& CV_SUCCESS == CVodeGetCurrentOrder(iData->cvode_mem, &q) && q >= 2);
	}
	
	if (performPredictorCompensation) {
		FMIStatus status;
		int cvStatus;
		size_t smooth_index = 0;
		FMIReal* delta_udot = (FMIReal*) N_VGetArrayPointer(iData->delta_udot);
		Statistics* stat = &(comp->iData->stat);

		/* Compute delta_udot, the jump in the input derivative. Note that df/du is to be computed at the old values of the input. */
		performPredictorCompensation = FMIFalse;
		for (i = 0; i < comp->nIn; ++i) {
			if (comp->ipData->inputUsesSmoother[i]) {
				/* Compute delta_udot, the jump in the input derivative. */
				const FMIReal udot_new = (ipData->inputForSmoother[i] - ipData->inputsT0[i]) / communicationStepSize;
				delta_udot[smooth_index] = udot_new - ipData->inputDerivatives[i];
				if (!performPredictorCompensation) {
					/* If delta_udot is zero, then we can skip the directional derivative. This happens e.g. if no inputs changed. It also happens
					when the inputs change linearly. We shouldn't include a "nominal" 1.0 in the Dymola_max as that would ruin the check for
					small scale inputs. It is better that performPredictorCompensation is set to true too often than the other way around. */
					const FMIReal udot_new_abs = Dymola_abs(udot_new);
					const FMIReal udot_old_abs = Dymola_abs(ipData->inputDerivatives[i]);
					if (Dymola_abs(delta_udot[smooth_index]) > 100.0*DBL_EPSILON*Dymola_max(udot_new_abs, udot_old_abs))
						performPredictorCompensation = FMITrue;
				}
				++smooth_index;
			}
		}

		/* If all of the requirements are fulfilled, compute the directional derivative and perform the compensation. */
		if (performPredictorCompensation) {
#if (FMI_VERSION >= FMI_2)
			const int cachingEnabled = GetAdditionalFlags(34);
			const int previousIcall = comp->icall;
			FMIBoolean restoringCache = cachingEnabled && previousIcall >= iDemandDerivative;
			FMIStatus statusFMUState;

			/* Copy FMU state to avoid invalidating the cache. */
			if (restoringCache) {
				statusFMUState = fmiGetFMUstate_(comp, &iData->save_state);
				if (statusFMUState != FMIOK) 
					restoringCache = FMIFalse;
			}
#endif

			/* Compute df/du * delta_udot. */
#if (FMI_VERSION == FMI_2)
			status = fmiGetDirectionalDerivative_(comp, iData->f_vrefs, comp->nStates, iData->u_vrefs, smooth_index, delta_udot, (FMIReal*) N_VGetArrayPointer(iData->df_du_delta_udot));
#elif (FMI_VERSION == FMI_3) 
			status = fmi3GetDirectionalDerivative_(comp, iData->f_vrefs, comp->nStates, iData->u_vrefs, smooth_index, delta_udot, smooth_index, (FMIReal*)N_VGetArrayPointer(iData->df_du_delta_udot), comp->nStates);
#else
			util_logger(comp, comp->instanceName, FMIFatal, loggBadCall, "Predictor compensation is only supported for FMI 2 and FMI 3.");
			return FMIFatal;
#endif
			stat->predictorCompensationFCalls += 2;
			comp->icall = iDemandStart;

#if (FMI_VERSION >= FMI_2)
			/* Restore FMU state. */
			if (restoringCache) {
				statusFMUState = fmiSetFMUstate_(comp, iData->save_state, FMIFalse);
				if (statusFMUState != FMIOK) 
					return statusFMUState;
			}
#endif

			/* If error then fail. If FMIDiscard then just skip the predictor compensation. */
			if (status == FMIError || status == FMIFatal)
				return status;

			/* Update solver history arrays to compensate for the jump in udot. */
			if (status == FMIOK || status == FMIWarning) {
#if INCLUDE_SUNDIALS_IDA
				if (IDA) {
					/* Update the IDA history arrays to compensate for the jump in udot. */
					cvStatus = DSIDAUpdateHistoryXbis(iData->cvode_mem, iData->df_du_delta_udot);
					if (cvStatus != IDA_SUCCESS)
						return FMIError;
				} else 
#endif
				{
					/* Update the xbis array of the Nordsieck history array to compensate for the jump in udot. */
					cvStatus = DSCVodeUpdateNordsieckXbis(iData->cvode_mem, iData->df_du_delta_udot);
					if (cvStatus != CV_SUCCESS)
						return FMIError;
				}
			}
		}
	}

	/* Input smoother: Part 2. */

	if (comp->useInputSmoother == 2) {
		/* Finally, compute the input derivative for the current doStep interval. We shouldn't update the inputs array as it should still be inputsT0 until the time is advanced. */
		for (i = 0; i < comp->nIn; ++i) {
			if (comp->ipData->inputUsesSmoother[i]) {
				comp->inputs[i] = ipData->inputsT0[i];
				ipData->inputDerivatives[i] = (ipData->inputForSmoother[i] - ipData->inputsT0[i]) / communicationStepSize;
			}
		}
	} else if (comp->useInputSmoother == 1) {
		/* Zero-order hold option. Immediately jump to new input values, but do not trigger a reset of the integrator */
		for (i = 0; i < comp->nIn; ++i) {
			if (comp->ipData->inputUsesSmoother[i]) {
				comp->inputs[i] = ipData->inputsT0[i] = ipData->inputForSmoother[i];
				ipData->inputDerivatives[i] = 0.0;
			}
		}
	} else
		return FMIError; /* Shouldn't happen */

	ipData->useDerivatives = 1;
	ipData->derivativeTime = currentCommunicationPoint;

	return FMIOK;
}

/* Copy the current inputs as starting point for the next interpolation. As we are at the end of the step, the interpolation has just reached
   the current inputs. */
DYMOLA_STATIC FMIStatus InputSmoother_UpdateOldInputs(Component* comp, const FMIReal endStepTime)
{
	size_t i;

	if (!comp || !comp->useInputSmoother || !comp->inputs || !comp->ipData || !comp->ipData->inputsT0 || comp->time != endStepTime)
		return FMIError;

	/* To make sure that the inputs array is reset. */
	integration_extrapolate_inputs(comp, endStepTime);

	/* Copy inputs. */
	for (i = 0; i < comp->nIn; ++i)
		comp->ipData->inputsT0[i] = comp->inputs[i];

	return FMIOK;
}

/* ------------------------------------------------------ */

static IntegrationStatus free_data(Component* comp, Component* target)
{
	IntegrationData* iData;

	if (target == NULL || target->iData == NULL) {
		return integrationOK;
	}
	iData = target->iData;

	if (iData->cvode_mem)
	{ 
#ifdef INCLUDE_SUNDIALS_IDA
		if (IDA) {
			IDAFree(&iData->cvode_mem);
		} else 
#endif		
		{
			CVodeFree(&iData->cvode_mem);
		}
		if (iData->A)
			SUNMatDestroy(iData->A);
		iData->A = NULL;
		if (iData->partialI)
			SUNMatDestroy(iData->partialI);
		iData->partialI = NULL;
		if (iData->linearSolver) {
#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
			DYNPropagateDidToThread(comp->did);
			EXCEPTION_TRY
			if (iData->sparseData.nnz && iData->sparseData.functions.ds_superlumt_free_global_data)
				(*iData->sparseData.functions.ds_superlumt_free_global_data)(iData->linearSolver);
			EXCEPTION_CATCH_ALL
				return integrationFatal;
			EXCEPTION_END;
#endif
			SUNLinSolFree(iData->linearSolver);
		}
		iData->linearSolver = NULL;
	}

	if (iData->y != NULL)      N_VDestroy_Serial(target->iData->y);
	if (iData->yDot != NULL)   N_VDestroy_Serial(target->iData->yDot);
	if (iData->abstol != NULL) N_VDestroy_Serial(target->iData->abstol);
	if (iData->reltol != NULL) N_VDestroy_Serial(target->iData->reltol);
	if (iData->ewt != NULL)    N_VDestroy_Serial(target->iData->ewt);
	if (iData->tmp1 != NULL)   N_VDestroy_Serial(target->iData->tmp1);
	if (iData->tmp2 != NULL)   N_VDestroy_Serial(target->iData->tmp2);
	if (iData->tmp3 != NULL)   N_VDestroy_Serial(target->iData->tmp3);

	if (iData->sunctx)
		SUNContext_Free(&iData->sunctx);
	iData->sunctx = NULL;

	if (iData->delta_udot) N_VDestroy_Serial(iData->delta_udot);
	if (iData->df_du_delta_udot) N_VDestroy_Serial(iData->df_du_delta_udot);
	if (iData->f_vrefs) FREE_MEMORY(iData->f_vrefs);
	if (iData->u_vrefs) FREE_MEMORY(iData->u_vrefs);
#if (FMI_VERSION >= FMI_2)
	if (iData->save_state) fmiFreeFMUstate_(comp, &iData->save_state);
#endif

	if (iData->sparseData.nnz) {
		if (iData->sparseData.colPtrs) FREE_MEMORY(iData->sparseData.colPtrs);
#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
		if (iData->sparseAnalyticData) FreeSparseAnalyticData(iData);
		iData->sparseAnalyticData = NULL;
#endif
		memset(&iData->sparseData,0,sizeof(iData->sparseData));
	}
#ifndef FMU_SOURCE_CODE_EXPORT
	if (iData->steadyStateUtils) {
		extern void FreeSteadyStateUtils(struct SteadyStateUtils* utils);
		FreeSteadyStateUtils(iData->steadyStateUtils);
		iData->steadyStateUtils = NULL;
	}
	if (comp->istruct && comp->istruct->mPrintEvent & (1 << 15))
		errCountDeAllocate();
#endif
	FREE_MEMORY(target->iData);
	return integrationOK;
}

/* ------------------------------------------------------ */
static int g(realtype t, N_Vector y, realtype *gout, void *user_data)
{
	FMIComponent* c = (FMIComponent*) user_data;
	Component* comp = (Component*) c;
	int retval = 0;

	/* temporarily change time, states and inputs to be able to reuse fmiGetEventIndicators */
	FMIReal time = comp->time;
	FMIReal* states = comp->states;

	assert(comp->nStates > 0);
	comp->states = N_VGetArrayPointer(y);

	integration_extrapolate_inputs(comp, t);

	comp->time = t;
	comp->icall = 0;
	if (fmiGetEventIndicators_(c, gout, comp->nCross) != FMIOK) {
		retval= -1;
	}

	/* restore time, states and inputs */
	integration_extrapolate_inputs(comp, time);
	comp->states = states;
	comp->time = time;
	return retval;
}

#ifdef INCLUDE_SUNDIALS_IDA
/* ------------------------------------------------------ */
static int g_IDA(realtype t, N_Vector y, N_Vector ydot, double* gout, void* user_data) 
{
	FMIComponent* c = (FMIComponent*) user_data;
	Component* comp = (Component*) c;
	realtype* ydot_data = N_VGetArrayPointer(ydot);
	realtype* iData_tmp3_data = N_VGetArrayPointer(comp->iData->tmp3);
	size_t i;
	int retval;

	if (comp->DAEMode) {
		if (comp->iData->DAEMinorEventRestart && comp->iData->DAEMinorEventRestartTime == t) {
			/* To avoid chattering at minor events in DAE mode use accurate crossing function values from the event iterations when evaluating g0 */
			memcpy(gout, comp->crossingFunctions, comp->nCross * sizeof(FMIReal));
			comp->iData->DAEMinorEventRestart = FMIFalse;
			comp->iData->DAEMinorEventRestartTime = 0.0;
			return 0;
		}

		if (comp->DAEDerivatives) {
			for (i = 0; i < comp->nStates; ++i)
				comp->DAEDerivatives[i] = (FMIReal) ydot_data[i]; /* Initialze DAEDerivatives for DAEmode */
		}
	}

	for (i = 0; i < comp->nStates; ++i)
		iData_tmp3_data[i] = ydot_data[i]; /* Temporarily store ydot in tmp3 and use for dy computation */

	retval = g(t, y, gout, user_data);

	for (i = 0; i < comp->nStates; ++i)
		ydot_data[i] = iData_tmp3_data[i]; /* Restore the IDA ydot approximation */

	return retval;
}
#endif

/* ------------------------------------------------------ */
static int check_step_event(void* user_data) {
	Component* comp = (Component*) user_data;
	comp->istruct->mBlockUnblockSmoothCrossings = 2;
	return (comp->istruct->mTriggerStepEvent != 0) || steady_state_reached(comp) || max_run_time_reached(comp);
}

/* ------------------------------------------------------ */
static void err_msg_handler(int error_code, const char *module, const char *function,
					 char *msg, void *eh_data)
{
	Component* comp = (Component*) eh_data;
	char* CVodeFlagName;
#ifdef INCLUDE_SUNDIALS_IDA
	if (IDA) {
		CVodeFlagName = IDAGetReturnFlagName(error_code);
	} else 
#endif	
	{
		CVodeFlagName = CVodeGetReturnFlagName(error_code);
	}
	util_logger(comp, comp->instanceName, FMIWarning, loggUndef, "%s: %s failed with %s:\n %s",
		module, function,  CVodeFlagName, msg);
	free(CVodeFlagName);
}

/* ------------------------------------------------------ */
/*
* cvEwtSetVV
*
* Function used to set the weight vector for the weighted norm. This allows elementwise relTol. Weights are set according to:
*
*	struct CVGodessData* comp = (struct CVGodessData*) user_data;
*  	ewt[i] = 1 / (comp->relTol[i] * SUNRabs(y[i]) + comp->absTol[i]), i=0,...,neq-1.
*
* - Erik Henningsson, 2017-05-30
*/
static int cvEwtSetVV(N_Vector y, N_Vector ewt, void *user_data) {

	Component* comp = (Component*) user_data; // user_data points to Component here
	IntegrationData* iData = comp->iData;
	int n;

	if (iData->abstol == NULL || iData->reltol == NULL) return -1;
	n = NV_LENGTH_S(y);
	if (NV_LENGTH_S(ewt) != n || NV_LENGTH_S(iData->reltol) != n || NV_LENGTH_S(iData->abstol) != n) return -1;

	N_VAbs(y, ewt);
	N_VProd(iData->reltol, ewt, ewt);
	N_VLinearSum(1.0, ewt, 1.0, iData->abstol, ewt);
	if (N_VMin(ewt) <= 0.0) return -1;
	N_VInv(ewt, ewt);
	return 0;
}

/* ------------------------------------------------------ */

#ifndef FMU_SOURCE_CODE_EXPORT
static int PrepareSteadyStateDerivativesToSkip(Component* comp)
{
	if (comp && comp->iData && comp->basetype) {

		extern int HasInitializedSteadyStateDerivativesToSkip_Internal(struct SteadyStateUtils* utils);
		extern int PrepareSteadyStateDerivativesToSkip_Internal(const int n, int* const ignored_derivatives, 
																const int has_ignored_derivatives, struct SteadyStateUtils* utils);

		const size_t n = comp->nStates;
		struct SteadyStateUtils* utils = comp->iData->steadyStateUtils;

		if (HasInitializedSteadyStateDerivativesToSkip_Internal(utils))
			return PrepareSteadyStateDerivativesToSkip_Internal((int) n, NULL, 0, utils);

		else {
			size_t i;
			int has_ignored_derivatives = 0;

			int* ignored_derivatives = (int*) ALLOCATE_MEMORY(n, sizeof(int));
			if (!ignored_derivatives) return 0;

			for (i = 0; i < n; ++i) {
				if (comp->basetype[i] & dsIsLowOrder) {
					ignored_derivatives[i] = 1;
					has_ignored_derivatives = 1;
				} else
					ignored_derivatives[i] = 0;
			}

			return PrepareSteadyStateDerivativesToSkip_Internal((int) n, ignored_derivatives, -has_ignored_derivatives, utils);
		}
	}

	return 0;
}
#endif

static int steady_state_reached(Component* comp) {
	if (!comp || GetAdditionalFlags(22))
		return FMIFalse;
	else if (comp->steadyStateReached)
		return FMITrue;
	else {
		const double detection_start_time = GetAdditionalReals(5);
		if (comp->time < detection_start_time)
			return FMIFalse;
	}
	if (GetAdditionalFlags(9) && comp->iData) {
		IntegrationData* iData = comp->iData;
		double* abstol_data = N_VGetArrayPointer(iData->abstol);
		double* reltol_data = N_VGetArrayPointer(iData->reltol);
		if (abstol_data && reltol_data) {
			const double tolerance_flag = GetAdditionalReals(2);
			const double default_dynamic_tolerance = GetAdditionalFlags(15) ? 1.0e-4 : 2.0e-2;
			const double relativeTolerance = comp->relativeToleranceDefined ? comp->relativeTolerance : cvodeTolerance;
#ifdef FMU_SOURCE_CODE_EXPORT
			const double tolerance = (tolerance_flag > 0 ? tolerance_flag : (default_dynamic_tolerance / (comp->tStop - comp->tStart)));
			const double quotient = tolerance / (relativeTolerance > 0.0 ? relativeTolerance : 0.0001) / 2.0;
			unsigned i;
			for (i = 0; i < comp->nStates; ++i) {
				const double limit = quotient * (abstol_data[i] + Dymola_abs(comp->states[i]) * reltol_data[i]);
				if (Dymola_abs(comp->derivatives[i]) > limit)
					return comp->steadyStateReached = FMIFalse;
			}
			return comp->steadyStateReached = FMITrue;
#else		
			{
				extern struct SteadyStateUtils* InitializeSteadyStateUtils(const int n, void* (*alloc_ptr)(size_t, size_t),
																		   void (*dealloc_ptr)(void*), const int num_closing_in_required);
				extern int CheckSteadyState_Internal(const int static_simulation, const int final_call,
													 const int has_ignored_derivatives, const double tolerance_flag,
													 const double default_dynamic_tolerance, const double t0,
													 const double tend, const double interval,
													 const double tol, const double* const x,
													 const double* const xd, const double* const scale,
													 const double* const abs_tol, const double* const rel_tol,
													 const int n, const int steady_state_reached,
													 struct SteadyStateUtils* utils, const double time);
				if (!iData->steadyStateUtils)
					iData->steadyStateUtils = InitializeSteadyStateUtils((int) comp->nStates, ALLOCATE_MEMORY_PTR, FREE_MEMORY_PTR, GetAdditionalFlags(9) ? 1 : 0);
				{
					const int hasIgnoredDerivatives = GetAdditionalFlags(21) && PrepareSteadyStateDerivativesToSkip(comp);
					return comp->steadyStateReached = CheckSteadyState_Internal(0, 0, hasIgnoredDerivatives, tolerance_flag,
						default_dynamic_tolerance, comp->tStart, comp->tStop, 0.0, relativeTolerance, comp->states, comp->derivatives,
						NULL, abstol_data, reltol_data, (int) comp->nStates, comp->steadyStateReached, iData->steadyStateUtils, comp->time);
				}
			}
#endif
		}
	}
	return comp->steadyStateReached = FMIFalse;
}

/* ------------------------------------------------------ */
static int max_run_time_reached(Component* comp) {
	if (!comp) return FMIFalse;
#ifndef FMU_SOURCE_CODE_EXPORT
	if (!comp->maxRunTimeReached) {
		const double max_run_time = GetAdditionalReals(4);
		if (max_run_time > 0.0) {
			extern double dymosimtime2_(int action, double* tbegin);
			if (dymosimtime2_(1, &comp->oneSimulationTotalTimerStart) >= max_run_time)
				return comp->maxRunTimeReached = FMITrue;
		}
	}
#endif
	return comp->maxRunTimeReached = FMIFalse;
}

static void InitializeSparseFixData(SparseFixData* sparseData)
{
	if (!sparseData) return;

	sparseData->nnz = sparseData->nGroups = sparseData->nprocs = sparseData->sparseEnabledVar = 0;

	sparseData->functions.sun_sparse_matrix = NULL;
	sparseData->functions.sun_linsol_superlumt = NULL;
	sparseData->functions.sun_sparse_matrix_index_pointers = NULL;
	sparseData->functions.sun_sparse_matrix_index_values = NULL;
	sparseData->functions.sun_sparse_matrix_data = NULL;
	sparseData->functions.ds_superlumt_free_global_data = NULL;
	sparseData->functions.ds_superlumt_reinit = NULL;
	sparseData->functions.ds_sunmat_add_diagonal_sparse = NULL;
	sparseData->functions.ds_sunmat_add_partial_diagonal_sparse = NULL;
	sparseData->functions.ds_sparse_analytic_jacobian_fix_data_allocate = NULL;
	sparseData->functions.ds_sparse_analytic_jacobian_fix_data_copy = NULL;
	sparseData->functions.ds_sparse_analytic_jacobian_fix_data_free = NULL;
	sparseData->functions.ds_sparse_set_analytic_jacobian_structure = NULL;
	sparseData->functions.ds_sparse_reset_analytic_jacobian_structure = NULL;
	sparseData->functions.ds_sparse_set_analytic_jacobian_values = NULL;
}

static void CopySparseFixData(const size_t nStates, SparseFixData* target, const SparseFixData* source)
{
	if (!target || !source) return;

	target->nnz = source->nnz;
	target->nGroups = source->nGroups;
	target->nprocs = source->nprocs;
	target->sparseEnabledVar = source->sparseEnabledVar;
	if (source->nnz) {
		size_t i;
		if (source->colPtrs && target->colPtrs) {
			for (i = 0; i < nStates+1; ++i)
				target->colPtrs[i] = source->colPtrs[i];
		}
	}
}

#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
/* checks if sparse solver is to be used and returns number of nonzero elements */
static int EnableSparse(int nx, SparseFixData *sparseData, int cgOffset, int gcOffset) {
	int nGroups = QJacobianCG_[cgOffset];
	int sparse_enabled = 0;
	if (!sparseData)
		return 0;
	sparse_enabled = GetAdditionalFlags(1);
	sparseData->sparseEnabledVar = sparse_enabled;
	if (!sparse_enabled)
		return 0;
	if (nGroups > 0) {
		// Count non-zero elements
		int i;
		int group_nr, group_ix = 1;
		int nz = 0;
		for (group_nr = 0; group_nr < nGroups; group_nr++) {
			int nmembers = QJacobianCG_[cgOffset + group_ix];
			const struct QJacobianTag_ pair = QJacobianGC2_[gcOffset + group_nr];
			const int * const b = pair.b;
			int structuredGroup = 0;
			if (nmembers < 0) {
				nmembers = 0;
				structuredGroup = 1;
			}
			if (pair.a == 0) {
				for (i = 0; i < nx; ++i) if (b[i] > 0) ++nz;
			}
			else if (pair.a == 1) {
				int ind = 1;
				for (i = 0; i < b[0]; ++i) {
					int num_rows = 0;
					++ind;
					num_rows = b[ind++];
					nz += num_rows;
					ind += num_rows;
				}
			} else if (structuredGroup && pair.a == 11) {
				for (int groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
					for (int column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
						for (int band = 0; band < b[groupMember + 3]; ++band) {
							int row = b[groupMember + 4 + band * 3 + 2] + (column - b[groupMember]);
							if (row >= b[groupMember + 4 + band * 3 + 0] && row < b[groupMember + 4 + band * 3 + 1]) {
								++nz; /* Could optimize further*/
							}
						}
					}
				}
			}
			group_ix += 1 + nmembers;
		}
		return nz;
	} else
		return GetAdditionalFlags(31);
}

/* computes fixed Jacobian structure data to be used when calculating numeric Jacobian using grouping */
static int PrepareSparse(Component *comp, SparseFixData *sparseData, int N, int nz, int cgOffset, int gcOffset) {
	int band_width = 0;
	int nGroups = QJacobianCG_[cgOffset];

	sparseData->nnz = nz;
	sparseData->nGroups = nGroups;
	sparseData->colPtrs = (int *) ALLOCATE_MEMORY(N+1, sizeof(int));

	{
		int nnz=0;
		int group_nr;
		int group_ix=1;
		{
			int j;
			for (j = 0; j < N + 1; j++) sparseData->colPtrs[j] = 0;
		}
		/* Find group */
		for (group_nr = 0; group_nr < nGroups; group_nr++) {
			int i;
			int nmembers = QJacobianCG_[cgOffset + group_ix];
			const struct QJacobianTag_ pair = QJacobianGC2_[gcOffset + group_nr];
			const int * const b = pair.b;
			int structuredGroup = 0;
			if (nmembers < 0) {
				nmembers = 0;
				structuredGroup = 1;
			}
			if (structuredGroup) {
				int groupMember, column, row;
				/* This is now efficient, see #1992 */
				for (groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
					for (column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
						int col = column - 1;
						int band;
						for (band = 0; band < b[groupMember + 3]; ++band) {
							row = b[groupMember + 4 + band * 3 + 2] + (column - b[groupMember]);
							if (row >= b[groupMember + 4 + band * 3 + 0] && row < b[groupMember + 4 + band * 3 + 1]) {
								int i = row - 1;

								sparseData->colPtrs[col]++;
								++nnz;
								band_width = Dymola_max(band_width, Dymola_abs(i - col));
							}
						}
					}
				}
			} else {
				/* Found group containing it, copy specific values */
				if (pair.a == 0) {
					for (i = 0; i < N; i++) {
						int col = b[i] - 1;
						if (col >= 0) {

							sparseData->colPtrs[col]++;
							++nnz;
							band_width = Dymola_max(band_width, Dymola_abs(i - col));
						}
					}
				} else if (pair.a == 1) {
					int j2, ind = 1;
					for (i = 0; i < b[0]; ++i) {
						int column, num_rows;
						column = b[ind++] - 1;
						num_rows = b[ind++];
						for (j2 = 0; j2 < num_rows; ++j2) {
							const int row = b[ind++] - 1;

							sparseData->colPtrs[column]++;
							++nnz;
							band_width = Dymola_max(band_width, Dymola_abs(row - column));
						}
					}
				}
			}
			group_ix += 1 + nmembers;
		}
		{
			int j, cumSum = 0;
			for (j = 0; j < N; j++) {
				int current = sparseData->colPtrs[j];
				sparseData->colPtrs[j] = cumSum;
				cumSum += current;
			}
		}
		sparseData->colPtrs[N]=nnz;
	}
	return band_width;
}

/* return the number of cores to be used by the multithreaded sparse SuperLU */
static int getNumProcsCvode(SparseFixData *sparseData, int nx, int nnz, int bw) {

	int np = 0;
	if (!sparseData)
		return 1;
	np = sparseData->sparseEnabledVar;
	if (np <= 0)
		return 1;
	else
		return np;
}

/* Allocate memory needed for sparse analytic Jacobian construction */
static int InitializeSparseAnalyticData(IntegrationData* iData, const int num_states, const int num_non_zero)
{
	SparseAnalyticData* sparseAnalyticData = NULL;
	if (!iData || iData->sparseAnalyticData) return 1;

	iData->sparseAnalyticData = (SparseAnalyticData*) ALLOCATE_MEMORY(1, sizeof(SparseAnalyticData));
	if (!iData->sparseAnalyticData) return 1;
	sparseAnalyticData = iData->sparseAnalyticData;
	sparseAnalyticData->analyticJacobianStructureInitialized = FMIFalse;

	if (!iData->sparseData.functions.ds_sparse_analytic_jacobian_fix_data_allocate
		|| (*iData->sparseData.functions.ds_sparse_analytic_jacobian_fix_data_allocate)(
			&sparseAnalyticData->fixData, num_states, num_non_zero, 1, ALLOCATE_MEMORY_PTR))
		return 1;

	sparseAnalyticData->JacCols = (int*) ALLOCATE_MEMORY(num_non_zero, sizeof(int));
	sparseAnalyticData->JacRows = (int*) ALLOCATE_MEMORY(num_non_zero, sizeof(int));
	sparseAnalyticData->JacValues = (double*) ALLOCATE_MEMORY(num_non_zero, sizeof(double));
	if (!sparseAnalyticData->JacCols || !sparseAnalyticData->JacRows || !sparseAnalyticData->JacValues) return 1;

	return 0;
}

/* Copy memory needed for sparse analytic Jacobian construction */
static void CopySparseAnalyticData(IntegrationData* iData_target, IntegrationData* iData_source)
{
	if (!iData_target || !iData_target->sparseAnalyticData || !iData_source || !iData_source->sparseAnalyticData) return;

	if (iData_source->sparseData.functions.ds_sparse_analytic_jacobian_fix_data_copy)
		(*iData_source->sparseData.functions.ds_sparse_analytic_jacobian_fix_data_copy)(
			iData_target->sparseAnalyticData->fixData, iData_source->sparseAnalyticData->fixData);

	iData_target->sparseAnalyticData->analyticJacobianStructureInitialized = iData_source->sparseAnalyticData->analyticJacobianStructureInitialized;

	/* No need to copy JacCols, JacRows, and JacValues. */
}

/* De-allocate memory needed for sparse analytic Jacobian construction */
static void FreeSparseAnalyticData(IntegrationData* iData) 
{
	SparseAnalyticData* sparseAnalyticData = NULL;
	if (!iData || !iData->sparseAnalyticData) return;
	sparseAnalyticData = iData->sparseAnalyticData;

	if (sparseAnalyticData->fixData && iData->sparseData.functions.ds_sparse_analytic_jacobian_fix_data_free) 
		(*iData->sparseData.functions.ds_sparse_analytic_jacobian_fix_data_free)(sparseAnalyticData->fixData, FREE_MEMORY_PTR);

	if (sparseAnalyticData->JacCols) FREE_MEMORY(sparseAnalyticData->JacCols);
	if (sparseAnalyticData->JacRows) FREE_MEMORY(sparseAnalyticData->JacRows);
	if (sparseAnalyticData->JacValues) FREE_MEMORY(sparseAnalyticData->JacValues);

	FREE_MEMORY(iData->sparseAnalyticData);
	iData->sparseAnalyticData = NULL;
}
#endif

/* Called by CVode after each successful step.Gives access to the internal memory structure for logging.
   Do not modify the CVode memory as leads to undefined behaviour. */
static int MonitorFn(void* cvode_mem, void* user_data)
{
	Component* comp = (Component*) user_data;
	int qlast = -1;
	int success;

	if (!comp || !comp->iData)
		return -1;

#ifdef INCLUDE_SUNDIALS_IDA
	if (IDA) {
		success = (IDA_SUCCESS == IDAGetLastOrder(cvode_mem, &qlast));
	} else 
#endif
	{
		success = (CV_SUCCESS == CVodeGetLastOrder(cvode_mem, &qlast));
	}

	if (success) {
		if (qlast > comp->iData->stat.maxOrder)
			comp->iData->stat.maxOrder = qlast;
	} else 
		return -1;

	return 0;
}

/* Enables 3ds accurate DAE event handling */
static int SetAccurateDAECrossing(void* user_data, int accurateCrossing)
{
	Component* comp = (Component*) user_data;
	if (!comp || !comp->istruct)
		return accurateCrossing;

	return comp->istruct->mAtDAEEvent = accurateCrossing;
}

/* For Simulator Performance (Simulation Analysis) integrator state error logging */
static void StateErrorMonitorFn(void* user_data, realtype time, realtype h, int q, int rejected,
	sunindextype N, realtype* ewt, realtype* e, realtype err, realtype mult)
{
#ifdef STORE_RESULT
	Component* comp = (Component*) user_data;
	if (comp) {
		const char* dummy = NULL;
		updateErrCount3((double) time, (double) h, q, rejected, (int) N, (double*) ewt, (double*) e, (double) err, (double) mult, 0, 
			comp->tStart, comp->storeResult && comp->result && comp->result->phase.xNames ? comp->result->phase.xNames : &dummy);
	}
#endif /* STORE_RESULT */
}

static void UpdateOffset(int* const cgOffset, int* const gcOffset)
{
	int i, j;
	if (!cgOffset || !gcOffset) return;
	*cgOffset = 0;
	*gcOffset = 0;
	for (i = 0; i < 2; ++i) {
		const int nGroups = QJacobianCG_[*cgOffset];
		*cgOffset += 1;
		for (j = 0; j < nGroups; ++j) {
			int grpSize = QJacobianCG_[*cgOffset];
			if (grpSize < 0) grpSize = 0;
			*cgOffset += 1 + grpSize;
		}
		*gcOffset += nGroups;
	}
}
