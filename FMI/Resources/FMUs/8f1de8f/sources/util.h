/* Utility header file for Dymosim FMI implementation. */

#ifndef util_h
#define util_h

/* need to include first so that correct files are included */
#include "conf.h"
#include "types.h"
#include <float.h>

/* ----------------- macros ----------------- */

/* Standard value reference */
/* idemand: 31-28, category: 27-24, index: 23-0 */
#define FMI_IDEMAND(valueRef) ((valueRef) >> 29)
#define FMI_CATEGORY(valueRef) (((valueRef) & 0x1f000000) >> 24)
#define FMI_INDEX(valueRef) ((valueRef) & 0xffffff)

/* Value reference for embedded systems */
/* idemand: 31-28, restricted: 27, FMI type size: 26-23, index: 22-0 */

#define MIN(A, B) ((A) <= (B) ? (A) : (B))

#define SMALL_TIME_DEV(curtime) (4 * DBL_EPSILON * (1 + fabs(curtime)))


#define HANDLE_STATUS_RETURN(status) return (status == FMIError) ? util_error(comp) : status;

#define ERROR_RETURN_CHECK(f)   \
switch(f){                      \
	case FMIOK:                 \
	case FMIWarning:            \
		break;                  \
	case FMIDiscard:            \
		return FMIDiscard;      \
	case FMIError:              \
		return util_error(comp);\
	case FMIFatal:              \
		return FMIFatal;        \
}

#ifdef MEMLEAK_DEBUG
#define ALLOCATE_MEMORY(nbr, size) memdebug_calloc(nbr, size)
#define FREE_MEMORY(ptr) do {memdebug_free(ptr); ptr=0;}while(0)
#define ALLOCATE_MEMORY_PTR (&memdebug_calloc)
#define FREE_MEMORY_PTR (&memdebug_free)
#else
#define ALLOCATE_MEMORY(nbr, size) calloc((nbr), (size))
#define FREE_MEMORY(ptr) do {free((void*) (ptr)); (ptr) = 0;}while(0)
#define ALLOCATE_MEMORY_PTR (&calloc)
#define FREE_MEMORY_PTR (&free)
#endif


/*TODO support larger string sizes and allocate more memory if needed*/
/* ------------------ function declarations ----------------- */

/* logger wrapper for handling off enabled/disabled logging */
DYMOLA_STATIC void util_logger(Component* comp, FMIString instanceName, FMIStatus status,
	                           FMIString category, FMIString message, ...);

#ifdef FMU_SOURCE_SINGLE_UNIT
/*log categories defined*/
static const char* loggFuncCall = "FunctionCall";
static const char* loggUndef = ""; /* For not yet specified log messages */
static const char* loggQiErr = "";
static const char* loggBadCall ="IllegalFunctionCall";
static const char* loggSundials ="Sundials";
static const char* loggMemErr = "memoryAllocation";
static const char* loggStats = "CvodeStatistics";
#else
extern const char* loggFuncCall;
extern const char* loggUndef;
extern const char* loggQiErr;
extern const char* loggBadCall;
extern const char* loggSundials;
extern const char* loggMemErr;
extern const char* loggStats;
#endif


/* buffered variant used when line breaks should be omitted */
DYMOLA_STATIC void util_buflogger(Component* comp, FMIString instanceName, FMIStatus status,
	                              FMIString category, FMIString message, ...);

/* cannot use strdup since direct use of malloc not allowed */
DYMOLA_STATIC FMIString util_strdup(FMIString s);

/* locally modified variant from Sundials to store in buffer instead of printing */
DYMOLA_STATIC int util_check_flag(void *flagvalue, char *funcname, int opt, Component* comp);

/* refresh variable values using dsblock_ */
DYMOLA_STATIC int util_refresh_cache(Component* comp, int idemand, const char* label, FMIBoolean* iterationConverged);

/* handle termination due to an error */
DYMOLA_STATIC FMIStatus util_error(Component* comp);

/* Initialize model, partly or completely, depending on argument "complete". */
DYMOLA_STATIC FMIStatus util_initialize_model(FMIComponent c, FMIBoolean toleranceControlled, FMIReal relativeTolerance, FMIBoolean complete);

/* Perform event iteration. */
DYMOLA_STATIC FMIStatus util_event_update(FMIComponent c, FMIBoolean intermediateResults,
#if (FMI_VERSION >= FMI_2)
/* needs another argument since not in eventInfo for FMI 2*/
FMIBoolean* terminateSolution
#else
FMIEventInfo* eventInfo
#endif
);

/* Initialize slave, partly or completely, depending on FMI version */
DYMOLA_STATIC FMIStatus util_initialize_slave(FMIComponent c, FMIReal relativeTolerance, FMIReal tStart, FMIBoolean StopTimeDefined, FMIReal tStop);

DYMOLA_STATIC void util_print_dymola_timers(FMIComponent c);

DYMOLA_STATIC void  unSuportedFunctionCall(FMIComponent c, FMIString funcName);

DYMOLA_STATIC FMIStatus checkSizes(FMIComponent c, const FMIValueReference * vr, size_t nVr, size_t nv, FMIString funcName, FMIString getSet);

#if (FMI_VERSION >= FMI_2)
/* Exit initialization mode. */
	DYMOLA_STATIC FMIStatus util_exit_model_initialization_mode(FMIComponent c, const char* label, ModelStatus nextStatus);

/* --------------------------------------------------------------------------------------------------------
API functions defined in earlier version of fmi 2.0 still in use internally
----------------------------------------------------------------------------------------------------------*/

/* Creation and destruction of model instances and setting debug status */
#if (FMI_VERSION >= FMI_3)
	DYMOLA_STATIC FMIStatus fmiSetupExperiment_(FMIComponent c, FMIBoolean relativeToleranceDefined, FMIReal relativeTolerance,	FMIReal tStart,	FMIBoolean tStopDefined, FMIReal tStop);
#else
	DYMOLA_STATIC FMIComponent fmiInstantiateModel_(FMIString, FMIString, FMIString, const FMICallbackFunctions*, FMIBoolean, FMIBoolean);
	DYMOLA_STATIC FMIComponent fmiInstantiateSlave_(FMIString, FMIString, FMIString, const FMICallbackFunctions*, FMIBoolean, FMIBoolean);
#endif
	DYMOLA_STATIC void fmiFreeModelInstance_(FMIComponent);

   /* Evaluation of the model equations */
	DYMOLA_STATIC FMIStatus fmiEnterModelInitializationMode_(FMIComponent, FMIBoolean, FMIReal);
	DYMOLA_STATIC FMIStatus fmiExitModelInitializationMode_(FMIComponent);
	DYMOLA_STATIC FMIStatus fmiTerminateModel_(FMIComponent);
	
	/* Creation and destruction of slave instances */
	DYMOLA_STATIC void fmiFreeSlaveInstance_(FMIComponent);

   /* Simulating the slave */
	DYMOLA_STATIC FMIStatus fmiEnterSlaveInitializationMode_(FMIComponent, FMIReal, FMIReal, FMIBoolean, FMIReal);
	DYMOLA_STATIC FMIStatus fmiExitSlaveInitializationMode_(FMIComponent);
	DYMOLA_STATIC FMIStatus fmiTerminateSlave_(FMIComponent);
	DYMOLA_STATIC FMIStatus fmiResetSlave_(FMIComponent);


#endif

#endif /* util_h */
