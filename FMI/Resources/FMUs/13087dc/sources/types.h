/* Internal types for Dymosim FMI implementation. */

#ifndef types_h
#define types_h

#include "conf.h"
#include "FMIversionPrefix.h"
#if (FMI_VERSION >= FMI_2) && !(defined(NO_FILE) || defined(RTTWINCAT))
/* for serialization of FMU state */
#include "tpl.h"
#endif /* FMI_2 */

#include <stdlib.h>

#define TIME_INFINITY 1e36
#if (FMI_VERSION >= FMI_3)
#include "fmi3PlatformTypes.h"
#include "fmi3FunctionTypes.h"
#elif (FMI_VERSION >= FMI_2)
#include "fmi2TypesPlatform.h"
#include "fmi2FunctionTypes.h"
#else
#include "fmiPlatformTypes_.h"
#include "fmiFunctions_.h"
#endif

/* for storing of result from within FMU */
#ifdef STORE_RESULT
#include "GenerateResultInNonDymosim.h"
#endif


#include "dsblock.h"
#include "delay.h"

#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
/* Sundials */
#include <nvector/nvector_serial.h>
#include <sundials/sundials_direct.h>
#include <sundials/sundials_context.h>
#include <sundials/sundials_matrix.h>
#include <sundials/sundials_linearsolver.h>
#include "ds_sundials_extensions/ds_sundials_extensions.h"
#endif /* ONLY_INCLUDE_INLINE_INTEGRATION */

/* ----------------- type definitions ----------------- */

#if (FMI_VERSION < FMI_2)
typedef char fmiChar;		/*fmiChar not included in FMI 1, required for String allocation*/
#endif

typedef enum {
	iDemandStart,            /* Start of integration: Initial equations are solved. */
	iDemandOutput,           /* Compute outputs */
	iDemandDerivative,       /* Compute derivatives */
	iDemandVariable,         /* Compute auxiliary variables */
	iDemandCrossingFunction, /* Compute crossing functions */
	iDemandEventHandling,    /* Event handling */
	iDemandTerminal = 7      /* 'terminal()' is true. */
} IDemandLevel;

typedef enum {
	modelInstantiated,
#if (FMI_VERSION >= FMI_3)
	modelConfigurationMode,
#endif
	modelInitializationMode, /*inside get macro and therfore required to be deifned for fmi1 */
#if (FMI_VERSION >= FMI_2)
	modelCsPreEventMode,
	modelEventMode,
	modelEventMode2,    /* event iteration ongoing */
	modelEventModeExit, /* event iteration finished */
#endif
	modelContinousTimeMode,
	modelTerminated
} ModelStatus;

typedef enum {
	integrationOK,
	integrationError,
	integrationTerminate,
	integrationEvent,
	integrationFatal
} IntegrationStatus;

typedef struct {
	long steps;
	long intSteps;
	int fCalls;
	int niters;
	int ncfails; 
	int ngevals;
	int njevals;
	float totTime;
	
	int rejSteps;
	int rejIntSteps;
	int rejFCalls;
	int rejJacCalls;

	int maxOrder;

	int predictorCompensationFCalls;
} Statistics;

typedef enum {
	fmiTypeFloat32,
	fmiTypeFloat64,
	fmiTypeInt8,
	fmiTypeUInt8,
	fmiTypeInt16,
	fmiTypeUInt16,
	fmiTypeInt32,
	fmiTypeUInt32,
	fmiTypeInt64,
	fmiTypeUInt64,
	fmiTypeBoolean,
	fmiTypeString,
	fmiTypeBineary,
	fmiTypeClock
}fmi3Types;

typedef union {
	FMIReal* rp;
	FMIInteger* ip;
	FMIBoolean* bp;
#if (FMI_VERSION > FMI_2)
	fmi3Int64* i64p;
	fmi3UInt64* ui64p;
#endif
} typePointer;

typedef union {
	const FMIReal* rp;
	const FMIInteger* ip;
	const FMIBoolean* bp;
#if (FMI_VERSION > FMI_2)
	const fmi3Int64* i64p;
	const fmi3UInt64* ui64p;
#endif
} ctypePointer;

#ifdef SAVE_STATE_SUNDIALS
#include "cvode_impl.h"
/* internal data necessary to expose due to lack of state saving support in API */
/* taken from CkpntMemRec */
typedef struct {
	/* Nordsieck History Array */
	N_Vector ck_zn[L_MAX];

	/* Step data */
	long int ck_nst;
	realtype ck_tretlast;
	int      ck_q;
	int      ck_qmax;
	int      ck_qprime;
	int      ck_qwait;
	int      ck_L;
	realtype ck_gammap;
	realtype ck_h;
	realtype ck_hprime;
	realtype ck_hscale;
	realtype ck_eta;
	realtype ck_etamax;
	realtype ck_tau[L_MAX+1];
	realtype ck_tq[NUM_TESTS+1];
	realtype ck_l[L_MAX];

	/* Saved values */
	realtype ck_saved_tq5;
} CVodeCheckPointData;
#endif

#ifndef ONLY_INCLUDE_INLINE_INTEGRATION

typedef struct {
	int *colPtrs;
	int nnz;
	int nGroups;
	int nprocs;
	SparseFunctions functions;
	int sparseEnabledVar;
} SparseFixData;

typedef struct {
	struct DSSparseAnalyticJacobianFixData* fixData;
	int* JacCols;
	int* JacRows;
	double* JacValues;
	FMIBoolean analyticJacobianStructureInitialized;
} SparseAnalyticData;
#endif

typedef struct {
#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
	N_Vector y;
	N_Vector yDot;
	void* cvode_mem;
#ifdef SAVE_STATE_SUNDIALS
	int cvode_called;
	CVodeCheckPointData ck_mem;
#endif /* SAVE_STATE_SUNDIALS */

	/* absolute tolerance */
	N_Vector abstol;
	/* relative tolerance */
	N_Vector reltol;
	/* error weight vector */
	N_Vector ewt;
	/* temporary space */
	N_Vector tmp1;
	N_Vector tmp2;
	N_Vector tmp3;

	SUNMatrix J;

	SparseFixData sparseData;

	struct SteadyStateUtils* steadyStateUtils;

	SUNContext sunctx;
	SUNMatrix A;
	SUNLinearSolver linearSolver;
	SUNMatrix partialI;
	FMIBoolean DAEMinorEventRestart;
	FMIReal DAEMinorEventRestartTime;

	/* Work array and fixed data for predictor compensation */
	N_Vector delta_udot;
	N_Vector df_du_delta_udot;
	FMIValueReference* f_vrefs;
	FMIValueReference* u_vrefs;
#if (FMI_VERSION >= FMI_2)
	FMIFMUstate save_state;
#endif

	SparseAnalyticData* sparseAnalyticData;

#endif /* ONLY_INCLUDE_INLINE_INTEGRATION */
	
	/* statistics */
	Statistics stat;
} IntegrationData;

/*InterpolationData  is removed from integration datat as it needs to be allocated during instantiation*/
typedef struct{ 
	/* for input derivatve handling */
	FMIReal derivativeTime;
	FMIReal* inputDerivatives;
	FMIReal* inputsT0;
	int useDerivatives;

	/* for output derivatve handling */
	FMIReal* outputsPrev;
	FMIReal timePrev;

	/* additional for input smoother */
	FMIBoolean* inputUsesSmoother;
	FMIReal* inputForSmoother;
}InterpolationData;

/* result storing on file */
typedef struct {
	FMIReal time;
	FMIReal* parameters;
	FMIReal* states;
	FMIReal* derivatives;
	FMIReal* outputs;
	FMIReal* inputs;
	FMIReal* auxiliary;
} ResultValues;

typedef struct {
#ifdef STORE_RESULT
	struct DeclarePhase phase;
#endif
	/* last time for sample, possibly buffered */
	FMIReal lastSampleTime;
	/* last time for sample on file */
	FMIReal lastFileSampleTime;
	/* last neglected sample warning applies up to this time */
	FMIReal lastWarnTime;
	/* buffered values */
	ResultValues** buf;
	int curBufSize;
	int curStartIx;
	int curIx;
	int nUsed;
	/* whether have warned about forced flush */
	FMIBoolean flushWarned;
	unsigned long long resultSampleCounter;
} Result;

#if (FMI_VERSION >= FMI_2)
/* for fmiGetDirectionalDerivative */
typedef struct {
	int nJacA;
	int nJacB;
	int nJacC;
	int nJacD;
	double * jacA;
	double * jacB;
	double * jacC;
	double * jacD;

	size_t nJacV;
	size_t nJacZ;
	double* jacV;
	double* jacVTmp1;
	double* jacVTmp2;
	double* jacZ;
	double* jacZTmp1;
	double* jacZTmp2;
	char*updatedJacobian;
	int nUpdatedJacobian;

	/*Temporary solution for discreteStates */
	FMIFMUstate stateStore; 
} JacobianData;
#endif /* FMI_2 */

typedef struct {
	size_t nbrReal;
	size_t nbrInt;
	size_t nbrStr;
	double *dim; //simplifes set and get code.
} DynArraySize;

typedef struct {
	FMIReal tStart;
	FMIBoolean StopTimeDefined;
	FMIReal tStop;
	FMIBoolean relativeToleranceDefined;
    FMIReal relativeTolerance;



	FMIBoolean isCoSim;

	FMIString instanceName;
	FMIBoolean loggingOn;
	FMIBoolean loggFuncCall;

	size_t nStates;
	size_t nIn;
	size_t nOut;
	size_t nAux;
	size_t nPar;
	size_t nSPar;
	size_t nCross;
	size_t nDStates;
	size_t nPrevious;
	size_t nConstAux;

	FMIReal* states;
	FMIReal* parameters;
	FMIReal* derivatives;
	FMIReal* inputs;
	FMIReal* outputs;
	FMIReal* auxiliary;
	FMIReal* crossingFunctions;
	FMIReal* dStates;
	FMIReal* previousVars;

	FMIReal* statesNominal;

	FMIChar** sParameters;
	FMIChar** tsParameters;

	FMIReal* oldStates; /*To check if state values have changed durin event*/

#if (FMI_VERSION >= FMI_3)
	size_t nRequiredIntermediateInputs, nRequiredIntermediateOutputs;
	fmi3IntermediateUpdateCallback intermediateUpdate;
	FMIBoolean earlyReturnAllowed, eventModeUsed;
#endif
#if (FMI_VERSION >= FMI_3)
	fmi3LogMessageCallback loggMessage;
	fmi3InstanceEnvironment ie;
#elif (FMI_VERSION >= FMI_2)
	const FMICallbackFunctions* functions;
#else
	FMICallbackFunctions functions;
#endif

	FMIReal time;
	int icall;

	/* for buffering of short messages without linebreaks */
	char logbuf[256];
	char* logbufp;

	ModelStatus mStatus;
	/* only for FMI 2.0 really, but convenient internally also for 1.0 */
	FMIBoolean terminationByModel;

	/* for storing of result from within FMU */
	FMIBoolean storeResult;
	Result* result;

	struct BasicDDymosimStruct* dstruct;
	struct BasicIDymosimStruct* istruct;
	/* to avoid numerous castings */
	double* duser;
	int* iuser;

	/* for internal integration */
	IntegrationData* iData;
	InterpolationData* ipData;

	int allocDone;
	int valWasSet;
	int reinitCvode;
	int eventIterRequired;
	int recalJacobian;
	int enforceRefresh; /*Temp due to idemand bug for discret states*/
	int hycosimInputEvent;

#if (FMI_VERSION >= FMI_2)
	/* for fmiGetDirectionalDerivative */
	JacobianData jacData;
	FMIBoolean firstEventCall;
#endif
	int eventIterationOnGoing;
	void**handles;
	int QiErr;
	struct DYNInstanceData* did;
	unsigned long long inlineStepCounter;
	double nextResultSampleTime;

	FMIBoolean AdvancedDiscreteStates;
	FMIBoolean* ClockZeroVector;
#if (FMI_VERSION >= FMI_2) && !(defined(NO_FILE) || defined(RTTWINCAT))
	/* for serialization using TPL */
	tpl_bin *tplDidBin;
	double * serializedDelayData;
#endif /* FMI_2 */

	FMIBoolean anyNonEvent;
	FMIBoolean steadyStateReached;

	FMIBoolean fmi2ComputeInit;

	FMIBoolean maxRunTimeReached;
	double oneSimulationTotalTimerStart;

	FMIInteger* basetype;

	FMIBoolean DAEMode;
	size_t nxOrig;
	FMIReal* DAEDerivatives;

	FMIInteger useInputSmoother;
	FMIBoolean usePredictorCompensation;

	DynArraySize dynArraySize;

} Component;

#ifndef DYMOLA_TIMES
#define DYMOLA_TIMES
struct DymolaTimes {
	int num;
	double maxim;
	double minim;
	double total;
	int numAccum;
	double maximAccum;
	double minimAccum;
	double totalAccum;
	const char*name;
	int mask;
};
#endif


#endif /* types_h */
