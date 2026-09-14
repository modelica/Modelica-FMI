#ifndef fmi3Import_h
#define fmi3Import_h
/* 
 *	Declare data structure used for import of FMUs in Modelica.
*/

/* For other platforms than those covered below, static linking of FMU binaries are expected to be used */
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#include "windows.h"

#else

typedef void* HINSTANCE;

#include "fmiImportUtils.h"

#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__) */

#undef false
#undef true

// used for FMU import with external library; for source code import this contains the #define FMI3_FUNCTION_PREFIX and for binary import
// it is empty
#if __has_include("_modelId.h") /* needed for scenarios without use of external library for import */
#include "_modelId.h"
#endif

#include "fmi3Functions.h"
#include "fmi3PlatformTypes.h"
#include "fmi3FunctionTypes.h"

#ifdef FMI3_FUNCTION_PREFIX
/* common */
fmi3SetDebugLoggingTYPE fmi3SetDebugLogging;
fmi3FreeInstanceTYPE fmi3FreeInstance;
fmi3EnterInitializationModeTYPE fmi3EnterInitializationMode;
fmi3ExitInitializationModeTYPE fmi3ExitInitializationMode;
fmi3EnterEventModeTYPE fmi3EnterEventMode;
fmi3TerminateTYPE fmi3Terminate;
fmi3ResetTYPE fmi3Reset;
fmi3GetFloat32TYPE fmi3GetFloat32;
fmi3GetFloat64TYPE fmi3GetFloat64;
fmi3GetInt8TYPE fmi3GetInt8;
fmi3GetUInt8TYPE fmi3GetUInt8;
fmi3GetInt16TYPE fmi3GetInt16;
fmi3GetUInt16TYPE fmi3GetUInt16;
fmi3GetInt32TYPE fmi3GetInt32;
fmi3GetUInt32TYPE fmi3GetUInt32;
fmi3GetInt64TYPE fmi3GetInt64;
fmi3GetUInt64TYPE fmi3GetUInt64;
fmi3GetBooleanTYPE fmi3GetBoolean;
fmi3GetStringTYPE fmi3GetString;
fmi3GetBinaryTYPE fmi3GetBinary;
fmi3GetClockTYPE fmi3GetClock;
fmi3SetFloat32TYPE fmi3SetFloat32;
fmi3SetFloat64TYPE fmi3SetFloat64;
fmi3SetInt8TYPE fmi3SetInt8;
fmi3SetUInt8TYPE fmi3SetUInt8;
fmi3SetInt16TYPE fmi3SetInt16;
fmi3SetUInt16TYPE fmi3SetUInt16;
fmi3SetInt32TYPE fmi3SetInt32;
fmi3SetUInt32TYPE fmi3SetUInt32;
fmi3SetInt64TYPE fmi3SetInt64;
fmi3SetUInt64TYPE fmi3SetUInt64;
fmi3SetBooleanTYPE fmi3SetBoolean;
fmi3SetStringTYPE fmi3SetString;
fmi3SetBinaryTYPE fmi3SetBinary;
fmi3SetClockTYPE fmi3SetClock;
fmi3GetFMUStateTYPE fmi3GetFMUState;
fmi3SetFMUStateTYPE fmi3SetFMUState;
fmi3FreeFMUStateTYPE fmi3FreeFMUState;
fmi3SerializedFMUStateSizeTYPE fmi3SerializedFMUStateSize;
fmi3SerializeFMUStateTYPE fmi3SerializeFMUState;
fmi3DeserializeFMUStateTYPE fmi3DeserializeFMUState;
fmi3EnterConfigurationModeTYPE fmi3EnterConfigurationMode;
fmi3ExitConfigurationModeTYPE fmi3ExitConfigurationMode;
fmi3UpdateDiscreteStatesTYPE fmi3UpdateDiscreteStates;
fmi3GetDirectionalDerivativeTYPE fmi3GetDirectionalDerivative;
/* model exchange */
fmi3InstantiateModelExchangeTYPE fmi3InstantiateModelExchange;
fmi3EnterContinuousTimeModeTYPE fmi3EnterContinuousTimeMode;
fmi3CompletedIntegratorStepTYPE fmi3CompletedIntegratorStep;
fmi3SetTimeTYPE fmi3SetTime;
fmi3SetContinuousStatesTYPE fmi3SetContinuousStates;
fmi3GetContinuousStateDerivativesTYPE fmi3GetContinuousStateDerivatives;
fmi3GetEventIndicatorsTYPE fmi3GetEventIndicators;
fmi3GetContinuousStatesTYPE fmi3GetContinuousStates;
/* co-simulation */
fmi3InstantiateCoSimulationTYPE fmi3InstantiateCoSimulation;
fmi3DoStepTYPE fmi3DoStep;
#endif // FMI3_FUNCTION_PREFIX

/*Common functions*/

typedef enum {dyfmi3InstantiationMode, dyfmi3InitializationMode, dyfmi3EventMode, dyfmi3ContinuousTimeMode, dyfmi3ConfigurationMode, dyfmi3ReConfigurationMode, dyfmi3TerminatedMode}dyfmi3mode;

struct dy_fmi3Extended;
struct dy_fmi3InputDer {
	struct dy_fmi3Extended *a;
	fmi3ValueReference inp;
	double val, derval, t;
};

struct dy_fmi3outptut {
	union {
		float* f;
		double* d;
	} vals;
	size_t nbrvals;
	fmi3ValueReference* vrs;
	size_t* pos;
	size_t nbrvars;
};

struct dy_fmi3Extended 
{
  fmi3Instance m;
  double dyTime;
  double dyLastTime;
  double dyFirstTimeEvent;
  int dyTriggered;
  dyfmi3mode currentMode;
  int discreteInputChanged;
  fmi3FMUState dyFMUstate;
  size_t dyFMUStateSize;
  fmi3Byte* dySerializeFMUstate;
  double dyLastStepTime;
  HINSTANCE hInst;
  struct dy_fmi3outptut outs64;
  struct dy_fmi3outptut outs32;
  struct dy_fmi3outptut sders;
  double* out;
  double* der;
  float* out32;
  int cpOut;
  int cpDer;
  int cpOut32;

  fmi3GetVersionTYPE* dyFmiGetVersion;

  fmi3InstantiateModelExchangeTYPE* dyFmiInstantiateModelExchange;
  fmi3InstantiateCoSimulationTYPE* dyFmiInstantiateCoSimulation;
  fmi3FreeInstanceTYPE* dyFmiFreeInstance;
  
  fmi3EnterInitializationModeTYPE* dyFmiEnterInitializationMode;
  fmi3ExitInitializationModeTYPE* dyFmiExitInitializationMode;
  fmi3TerminateTYPE* dyFmiTerminate;
  fmi3ResetTYPE* dyFmiReset;
  fmi3SetDebugLoggingTYPE* dyFmiSetDebugLogging;

  fmi3GetFloat32TYPE* dyFmiGetFloat32;
  fmi3GetFloat64TYPE* dyFmiGetFloat64;
  fmi3GetInt8TYPE* dyFmiGetInt8;
  fmi3GetInt16TYPE* dyFmiGetInt16;
  fmi3GetInt32TYPE* dyFmiGetInt32;
  fmi3GetInt64TYPE* dyFmiGetInt64;
  fmi3GetUInt8TYPE* dyFmiGetUInt8;
  fmi3GetUInt16TYPE* dyFmiGetUInt16;
  fmi3GetUInt32TYPE* dyFmiGetUInt32;
  fmi3GetUInt64TYPE* dyFmiGetUInt64;
  fmi3GetBooleanTYPE* dyFmiGetBoolean;
  fmi3GetStringTYPE* dyFmiGetString;
  fmi3GetBinaryTYPE* dyFmiGetBinary;
  fmi3GetClockTYPE* dyFmiGetClock;

  fmi3SetFloat32TYPE* dyFmiSetFloat32;
  fmi3SetFloat64TYPE* dyFmiSetFloat64;
  fmi3SetInt8TYPE* dyFmiSetInt8;
  fmi3SetInt16TYPE* dyFmiSetInt16;
  fmi3SetInt32TYPE* dyFmiSetInt32;
  fmi3SetInt64TYPE* dyFmiSetInt64;
  fmi3SetUInt8TYPE* dyFmiSetUInt8;
  fmi3SetUInt16TYPE* dyFmiSetUInt16;
  fmi3SetUInt32TYPE* dyFmiSetUInt32;
  fmi3SetUInt64TYPE* dyFmiSetUInt64;
  fmi3SetBooleanTYPE* dyFmiSetBoolean;
  fmi3SetStringTYPE* dyFmiSetString;
  fmi3SetBinaryTYPE* dyFmiSetBinary;
  fmi3SetClockTYPE* dyFmiSetClock;

  fmi3GetNumberOfVariableDependenciesTYPE* dyFMIGetNumberOfVariableDependencies;
  fmi3GetVariableDependenciesTYPE* dyFMIGetVariableDependencies;

  fmi3SetFMUStateTYPE* dyFmiSetFMUState;
  fmi3GetFMUStateTYPE*  dyFmiGetFMUState;
  fmi3FreeFMUStateTYPE* dyFmiFreeFMUState;
  fmi3SerializedFMUStateSizeTYPE* dyFmiSerializedFMUStateSize;
  fmi3SerializeFMUStateTYPE*  dyFmiSerializeFMUState;
  fmi3DeserializeFMUStateTYPE* dyFmiDeserializeFMUState;

  fmi3GetDirectionalDerivativeTYPE* dyFmiGetDirectionalDerivative;
  fmi3GetAdjointDerivativeTYPE* dyFmiGetAdjointDerivative;

  fmi3EnterConfigurationModeTYPE* dyFmiEnterConfigurationMode;
  fmi3ExitConfigurationModeTYPE* dyFmiExitConfigurationMode;


  fmi3SetTimeTYPE* dyFmiSetTime;
  fmi3SetContinuousStatesTYPE* dyFmiSetContinuousStates;
  fmi3GetContinuousStatesTYPE* dyFmiGetContinuousStates;
  fmi3CompletedIntegratorStepTYPE* dyFmiCompletedIntegratorStep;
  fmi3GetContinuousStateDerivativesTYPE* dyFmiGetContinuousStateDerivatives;
  fmi3GetEventIndicatorsTYPE* dyFmiGetEventIndicators;
  fmi3EnterEventModeTYPE* dyFmiEnterEventMode;
  fmi3UpdateDiscreteStatesTYPE* dyFmiUpdateDiscreteStates;
  fmi3EnterContinuousTimeModeTYPE* dyFmiEnterContinuousTimeMode;
  
  fmi3EnterStepModeTYPE* dyFmiEnterStepMode;
  fmi3DoStepTYPE* dyFmiDoStep;

  fmi3InstanceEnvironment* dyInstanceEnvironment;
  int nrInputWithDer;
}; 

#endif
