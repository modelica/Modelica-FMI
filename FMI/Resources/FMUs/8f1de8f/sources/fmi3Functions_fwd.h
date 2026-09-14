#ifndef fmi3Functions_3_0_fwd_h
#define fmi3Functions_3_0_fwd_h


#ifdef __cplusplus
extern "C" {
#endif
#include "fmi3PlatformTypes.h"
#include "fmi3FunctionTypes.h"
#include "fmiFunctions_.h"
#include <stdlib.h>

DYMOLA_STATIC fmi3GetVersionTYPE      fmiGetVersion_;
DYMOLA_STATIC fmi3SetDebugLoggingTYPE fmiSetDebugLogging_;

DYMOLA_STATIC fmi3InstantiateModelExchangeTYPE         fmi3InstantiateModelExchange_;
DYMOLA_STATIC fmi3InstantiateCoSimulationTYPE          fmi3InstantiateCoSimulation_;
DYMOLA_STATIC fmi3FreeInstanceTYPE                     fmiFreeInstance_;
DYMOLA_STATIC fmi3EnterInitializationModeTYPE fmi3EnterInitializationMode_;
DYMOLA_STATIC fmi3ExitInitializationModeTYPE  fmiExitInitializationMode_;
DYMOLA_STATIC fmi3EnterEventModeTYPE          fmiEnterEventMode_;
DYMOLA_STATIC fmi3TerminateTYPE               fmiTerminate_;
DYMOLA_STATIC fmi3ResetTYPE                   fmiReset_;

DYMOLA_STATIC fmi3GetFloat32TYPE fmi3GetFloat32_;
DYMOLA_STATIC fmi3GetFloat64TYPE fmi3GetFloat64_;
DYMOLA_STATIC fmi3GetInt8TYPE    fmi3GetInt8_;
DYMOLA_STATIC fmi3GetUInt8TYPE   fmi3GetUInt8_;
DYMOLA_STATIC fmi3GetInt16TYPE   fmi3GetInt16_;
DYMOLA_STATIC fmi3GetUInt16TYPE  fmi3GetUInt16_;
DYMOLA_STATIC fmi3GetInt32TYPE   fmi3GetInt32_;
DYMOLA_STATIC fmi3GetUInt32TYPE  fmi3GetUInt32_;
DYMOLA_STATIC fmi3GetInt64TYPE   fmi3GetInt64_;
DYMOLA_STATIC fmi3GetUInt64TYPE  fmi3GetUInt64_;
DYMOLA_STATIC fmi3GetBooleanTYPE fmi3GetBoolean_;
DYMOLA_STATIC fmi3GetStringTYPE  fmi3GetString_;
DYMOLA_STATIC fmi3GetBinaryTYPE  fmi3GetBinary_;
DYMOLA_STATIC fmi3SetClockTYPE   fmi3SetClock_;

DYMOLA_STATIC fmi3SetFloat32TYPE fmi3SetFloat32_;
DYMOLA_STATIC fmi3SetFloat64TYPE fmi3SetFloat64_;
DYMOLA_STATIC fmi3SetInt8TYPE    fmi3SetInt8_;
DYMOLA_STATIC fmi3SetUInt8TYPE   fmi3SetUInt8_;
DYMOLA_STATIC fmi3SetInt16TYPE   fmi3SetInt16_;
DYMOLA_STATIC fmi3SetUInt16TYPE  fmi3SetUInt16_;
DYMOLA_STATIC fmi3SetInt32TYPE   fmi3SetInt32_;
DYMOLA_STATIC fmi3SetUInt32TYPE  fmi3SetUInt32_;
DYMOLA_STATIC fmi3SetInt64TYPE   fmi3SetInt64_;
DYMOLA_STATIC fmi3SetUInt64TYPE  fmi3SetUInt64_;
DYMOLA_STATIC fmi3SetBooleanTYPE fmi3SetBoolean_;
DYMOLA_STATIC fmi3SetStringTYPE  fmi3SetString_;
DYMOLA_STATIC fmi3SetBinaryTYPE  fmi3SetBinary_;

DYMOLA_STATIC fmi3GetClockTYPE   fmi3GetClock_;

/* Creation and destruction of FMU instances */
DYMOLA_STATIC fmi3InstantiateScheduledExecutionTYPE    fmi3InstantiateScheduledExecution_;
/* Enter and exit initialization mode, terminate and reset */

/* Getting and setting variables values */

/* Getting Variable Dependency Information */
DYMOLA_STATIC fmi3GetNumberOfVariableDependenciesTYPE fmi3GetNumberOfVariableDependencies_;
DYMOLA_STATIC fmi3GetVariableDependenciesTYPE         fmi3GetVariableDependencies_;

/* Getting and setting the internal FMU state */
DYMOLA_STATIC fmi3GetFMUStateTYPE            fmiGetFMUstate_;
DYMOLA_STATIC fmi3Status                     fmiSetFMUstate_(fmi3Instance instance, fmi3FMUState FMUState, fmi3Boolean reinitIntegration);
DYMOLA_STATIC fmi3FreeFMUStateTYPE           fmiFreeFMUstate_;
DYMOLA_STATIC fmi3SerializedFMUStateSizeTYPE fmiSerializedFMUstateSize_;
DYMOLA_STATIC fmi3SerializeFMUStateTYPE      fmiSerializeFMUstate_;
DYMOLA_STATIC fmi3DeserializeFMUStateTYPE    fmiDeSerializeFMUstate_;

/* Getting partial derivatives */
DYMOLA_STATIC fmi3GetDirectionalDerivativeTYPE fmi3GetDirectionalDerivative_;
DYMOLA_STATIC fmi3GetAdjointDerivativeTYPE     fmi3GetAdjointDerivative_;

/* Entering and exiting the Configuration or Reconfiguration Mode */
DYMOLA_STATIC fmi3EnterConfigurationModeTYPE fmi3EnterConfigurationMode_;
DYMOLA_STATIC fmi3ExitConfigurationModeTYPE  fmi3ExitConfigurationMode_;

/* Clock related functions */
DYMOLA_STATIC fmi3GetIntervalDecimalTYPE     fmi3GetIntervalDecimal_;
DYMOLA_STATIC fmi3GetIntervalFractionTYPE    fmi3GetIntervalFraction_;
DYMOLA_STATIC fmi3GetShiftDecimalTYPE        fmi3GetShiftDecimal_;
DYMOLA_STATIC fmi3GetShiftFractionTYPE       fmi3GetShiftFraction_;
DYMOLA_STATIC fmi3SetIntervalDecimalTYPE     fmi3SetIntervalDecimal_;
DYMOLA_STATIC fmi3SetIntervalFractionTYPE    fmi3SetIntervalFraction_;
DYMOLA_STATIC fmi3SetShiftDecimalTYPE        fmi3SetShiftDecimal_;
DYMOLA_STATIC fmi3SetShiftFractionTYPE       fmi3SetShiftFraction_;

DYMOLA_STATIC fmi3EvaluateDiscreteStatesTYPE fmi3EvaluateDiscreteStates_;
DYMOLA_STATIC fmi3UpdateDiscreteStatesTYPE   fmi3UpdateDiscreteStates_;

/***************************************************
Functions for Model Exchange
****************************************************/

DYMOLA_STATIC fmi3EnterContinuousTimeModeTYPE fmiEnterContinuousTimeMode_;
DYMOLA_STATIC fmi3CompletedIntegratorStepTYPE fmiCompletedIntegratorStep_;

/* Providing independent variables and re-initialization of caching */
DYMOLA_STATIC fmi3SetTimeTYPE             fmiSetTime_;
DYMOLA_STATIC fmi3SetContinuousStatesTYPE fmiSetContinuousStates_;

/* Evaluation of the model equations */
DYMOLA_STATIC fmi3GetContinuousStateDerivativesTYPE fmiGetDerivatives_;
DYMOLA_STATIC fmi3GetEventIndicatorsTYPE            fmiGetEventIndicators_;
DYMOLA_STATIC fmi3GetContinuousStatesTYPE           fmiGetContinuousStates_;
DYMOLA_STATIC fmi3GetNominalsOfContinuousStatesTYPE fmi3GetNominalsOfContinuousStates_;
DYMOLA_STATIC fmi3GetNumberOfEventIndicatorsTYPE    fmi3GetNumberOfEventIndicators_;
DYMOLA_STATIC fmi3GetNumberOfContinuousStatesTYPE   fmi3GetNumberOfContinuousStates_;

/***************************************************
Functions for Co-Simulation
****************************************************/

/* Simulating the FMU */
DYMOLA_STATIC fmi3EnterStepModeTYPE          fmi3EnterStepMode_;
DYMOLA_STATIC fmi3GetOutputDerivativesTYPE   fmi3GetOutputDerivatives_;
DYMOLA_STATIC fmi3ActivateModelPartitionTYPE fmi3ActivateModelPartition_;
DYMOLA_STATIC fmi3DoStepTYPE                 fmi3DoStep_;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif
#endif /* fmi3Functions_h */
