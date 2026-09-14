#ifndef BUILDFMU
/* needed for source code export */
#include "conf.h"
#endif

/* Include stddef.h, in order that size_t etc. is defined */
#include <stddef.h>

#include "fmi3Functions_fwd.h"
#include "fmi3Functions.h"


const char* fmi3GetVersion(void) {
    return fmiGetVersion_();
}



fmi3Status fmi3SetDebugLogging(fmi3Instance instance,
    fmi3Boolean loggingOn,
    size_t nCategories,
    const fmi3String categories[]) {
    return fmiSetDebugLogging_(instance, loggingOn, nCategories, categories);
}


fmi3Instance fmi3InstantiateModelExchange(
    fmi3String                 instanceName,
    fmi3String                 instantiationToken,
    fmi3String                 resourcePath,
    fmi3Boolean                visible,
    fmi3Boolean                loggingOn,
    fmi3InstanceEnvironment    instanceEnvironment,
    fmi3LogMessageCallback     logMessage) {

	return fmi3InstantiateModelExchange_(instanceName, instantiationToken, resourcePath, visible, loggingOn, instanceEnvironment,logMessage);
}

fmi3Instance fmi3InstantiateCoSimulation(
    fmi3String                     instanceName,
    fmi3String                     instantiationToken,
    fmi3String                     resourcePath,
    fmi3Boolean                    visible,
    fmi3Boolean                    loggingOn,
    fmi3Boolean                    eventModeUsed,
    fmi3Boolean                    earlyReturnAllowed,
    const fmi3ValueReference       requiredIntermediateVariables[],
    size_t                         nRequiredIntermediateVariables,
    fmi3InstanceEnvironment        instanceEnvironment,
    fmi3LogMessageCallback         logMessage,
    fmi3IntermediateUpdateCallback intermediateUpdate) {

    return fmi3InstantiateCoSimulation_(instanceName, instantiationToken, resourcePath, visible, loggingOn, eventModeUsed, 
        earlyReturnAllowed, requiredIntermediateVariables, nRequiredIntermediateVariables, instanceEnvironment, logMessage, intermediateUpdate);
}

fmi3Instance fmi3InstantiateScheduledExecution(
    fmi3String                     instanceName,
    fmi3String                     instantiationToken,
    fmi3String                     resourcePath,
    fmi3Boolean                    visible,
    fmi3Boolean                    loggingOn,
    fmi3InstanceEnvironment        instanceEnvironment,
    fmi3LogMessageCallback         logMessage,
    fmi3ClockUpdateCallback        clockUpdate,
    fmi3LockPreemptionCallback     lockPreemption,
    fmi3UnlockPreemptionCallback   unlockPreemption) {
    return NULL;
}

void fmi3FreeInstance(fmi3Instance instance) {
    fmiFreeInstance_(instance);
 }

fmi3Status fmi3EnterInitializationMode(fmi3Instance instance,
    fmi3Boolean toleranceDefined,
    fmi3Float64 tolerance,
    fmi3Float64 startTime,
    fmi3Boolean stopTimeDefined,
    fmi3Float64 stopTime) {
     return fmi3EnterInitializationMode_(instance, toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime);
}



fmi3Status fmi3ExitInitializationMode(fmi3Instance instance) {
    return fmiExitInitializationMode_(instance);
  }


fmi3Status fmi3EnterEventMode(fmi3Instance instance) {
    return fmiEnterEventMode_(instance);
}

fmi3Status fmi3Terminate(fmi3Instance instance) {
    return fmiTerminate_(instance);
}


fmi3Status fmi3Reset(fmi3Instance instance) {
    return fmiReset_(instance);
}


fmi3Status fmi3GetFloat32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float32 values[],
    size_t nValues) {
    return fmi3GetFloat32_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetFloat64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float64 values[],
    size_t nValues) {
    return fmi3GetFloat64_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Int8 values[],
    size_t nValues) {
    return fmi3GetInt8_(instance,valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetUInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt8 values[],
    size_t nValues) {
    return fmi3GetUInt8_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Int16 values[],
    size_t nValues) {
    return fmi3GetInt16_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetUInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt16 values[],
    size_t nValues) {
    return fmi3GetUInt16_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Int32 values[],
    size_t nValues) {
    return fmi3GetInt32_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetUInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt32 values[],
    size_t nValues) {
    return fmi3GetUInt32_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Int64 values[],
    size_t nValues) {
    return fmi3GetInt64_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetUInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt64 values[],
    size_t nValues) {
    return fmi3GetUInt64_(instance, valueReferences, nValueReferences, values, nValues);
  }

fmi3Status fmi3GetBoolean(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Boolean values[],
    size_t nValues) {
    return fmi3GetBoolean_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetString(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3String values[],
    size_t nValues) {
    return fmi3GetString_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3GetBinary(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    size_t valueSizes[],
    fmi3Binary values[],
    size_t nValues) {
    return fmi3GetBinary_(instance, valueReferences, nValueReferences, valueSizes, values, nValues);
}


fmi3Status fmi3GetClock(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Clock values[]) {
    return fmi3GetClock_(instance, valueReferences, nValueReferences, values);
}

fmi3Status fmi3SetFloat32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float32 values[],
    size_t nValues) {
    return fmi3SetFloat32_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetFloat64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float64 values[],
    size_t nValues) {
    return fmi3SetFloat64_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int8 values[],
    size_t nValues) {
    return fmi3SetInt8_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetUInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt8 values[],
    size_t nValues) {
    return fmi3SetUInt8_(instance, valueReferences, nValueReferences, values, nValues);
 }

fmi3Status fmi3SetInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int16 values[],
    size_t nValues) {
    return fmi3SetInt16_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetUInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt16 values[],
    size_t nValues) {
    return fmi3SetUInt16_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int32 values[],
    size_t nValues) {
    return fmi3SetInt32_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetUInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt32 values[],
    size_t nValues) {
    return fmi3SetUInt32_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int64 values[],
    size_t nValues) {
    return fmi3SetInt64_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetUInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt64 values[],
    size_t nValues) {
    return fmi3SetUInt64_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetBoolean(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Boolean values[],
    size_t nValues) {
    return fmi3SetBoolean_(instance, valueReferences, nValueReferences, values, nValues);
 }

fmi3Status fmi3SetString(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3String values[],
    size_t nValues) {
    return fmi3SetString_(instance, valueReferences, nValueReferences, values, nValues);
}

fmi3Status fmi3SetBinary(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const size_t valueSizes[],
    const fmi3Binary values[],
    size_t nValues) {
    return fmi3SetBinary_(instance, valueReferences, nValueReferences, valueSizes, values, nValues);
}

fmi3Status fmi3SetClock(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Clock values[]) {
    return fmi3SetClock_(instance, valueReferences, nValueReferences, values);
}

fmi3Status fmi3GetNumberOfVariableDependencies(fmi3Instance instance,
    fmi3ValueReference valueReference,
    size_t* nDependencies) {
    return fmi3GetNumberOfVariableDependencies_(instance, valueReference, nDependencies);
 }


fmi3Status fmi3GetVariableDependencies(fmi3Instance instance,
    fmi3ValueReference dependent,
    size_t elementIndicesOfDependent[],
    fmi3ValueReference independents[],
    size_t elementIndicesOfIndependents[],
    fmi3DependencyKind dependencyKinds[],
    size_t nDependencies) {
    return fmi3GetVariableDependencies_(instance, dependent, elementIndicesOfDependent, independents, elementIndicesOfIndependents, dependencyKinds, nDependencies);
}


fmi3Status fmi3GetFMUState(fmi3Instance instance, fmi3FMUState* FMUState) {
    return fmiGetFMUstate_(instance, FMUState);
}


fmi3Status fmi3SetFMUState(fmi3Instance instance, fmi3FMUState  FMUState) {
    return fmiSetFMUstate_(instance, FMUState, fmi3True);
}


fmi3Status fmi3FreeFMUState(fmi3Instance instance, fmi3FMUState* FMUState) {
    return fmiFreeFMUstate_(instance, FMUState);
}

fmi3Status fmi3SerializedFMUStateSize(fmi3Instance instance,
    fmi3FMUState  FMUState,
    size_t* size) {
    return fmiSerializedFMUstateSize_(instance, FMUState, size);
};

fmi3Status fmi3SerializeFMUState(fmi3Instance instance,
    fmi3FMUState  FMUState,
    fmi3Byte serializedState[],
    size_t size) {
    return fmiSerializeFMUstate_(instance, FMUState, serializedState, size);
}

fmi3Status fmi3DeserializeFMUState(fmi3Instance instance,
    const fmi3Byte serializedState[],
    size_t size,
    fmi3FMUState* FMUState) {
    return fmiDeSerializeFMUstate_(instance, serializedState, size, FMUState);
}

fmi3Status fmi3GetDirectionalDerivative(fmi3Instance instance,
    const fmi3ValueReference unknowns[],
    size_t nUnknowns,
    const fmi3ValueReference knowns[],
    size_t nKnowns,
    const fmi3Float64 seed[],
    size_t nSeed,
    fmi3Float64 sensitivity[],
    size_t nSensitivity) {
    return fmi3GetDirectionalDerivative_(instance, unknowns, nUnknowns, knowns, nKnowns, seed, nSeed, sensitivity, nSensitivity);
}


fmi3Status fmi3GetAdjointDerivative(fmi3Instance instance,
    const fmi3ValueReference unknowns[],
    size_t nUnknowns,
    const fmi3ValueReference knowns[],
    size_t nKnowns,
    const fmi3Float64 seed[],
    size_t nSeed,
    fmi3Float64 sensitivity[],
    size_t nSensitivity) {
    return fmi3GetAdjointDerivative_(instance, unknowns, nUnknowns, knowns, nKnowns, seed, nSeed, sensitivity, nSensitivity);
}



fmi3Status fmi3EnterConfigurationMode(fmi3Instance instance) {
    return fmi3EnterConfigurationMode_(instance);
}

fmi3Status fmi3ExitConfigurationMode(fmi3Instance instance) {
    return fmi3ExitConfigurationMode_(instance);
}


fmi3Status fmi3GetIntervalDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float64 intervals[],
    fmi3IntervalQualifier qualifiers[]) {
    return fmi3GetIntervalDecimal_(instance, valueReferences, nValueReferences, intervals, qualifiers);
}

fmi3Status fmi3GetIntervalFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt64 intervalCounters[],
    fmi3UInt64 resolutions[],
    fmi3IntervalQualifier qualifiers[]) {
    return fmi3GetIntervalFraction_(instance, valueReferences, nValueReferences, intervalCounters, resolutions, qualifiers);
 }


fmi3Status fmi3GetShiftDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float64 shifts[]) {
    return fmi3GetShiftDecimal_(instance, valueReferences, nValueReferences, shifts);
}


fmi3Status fmi3GetShiftFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt64 shiftCounters[],
    fmi3UInt64 resolutions[]) {
    return fmi3GetShiftFraction_(instance, valueReferences, nValueReferences, shiftCounters, resolutions);
}


fmi3Status fmi3SetIntervalDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float64 intervals[]) {
    return fmi3SetIntervalDecimal_(instance, valueReferences, nValueReferences, intervals);
}

fmi3Status fmi3SetIntervalFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt64 intervalCounters[],
    const fmi3UInt64 resolutions[]) {
    return fmi3SetIntervalFraction_(instance, valueReferences, nValueReferences, intervalCounters, resolutions);
}


fmi3Status fmi3SetShiftDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float64 shifts[]) {
    return fmi3SetShiftDecimal_(instance, valueReferences, nValueReferences, shifts);
}


fmi3Status fmi3SetShiftFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt64 shiftCounters[],
    const fmi3UInt64 resolutions[]) {
    return fmi3SetShiftFraction_(instance, valueReferences, nValueReferences, shiftCounters, resolutions);
}



fmi3Status fmi3EvaluateDiscreteStates(fmi3Instance instance) {
    return fmi3EvaluateDiscreteStates_(instance);
}

fmi3Status fmi3UpdateDiscreteStates(fmi3Instance instance,
    fmi3Boolean* discreteStatesNeedUpdate,
    fmi3Boolean* terminateSimulation,
    fmi3Boolean* nominalsOfContinuousStatesChanged,
    fmi3Boolean* valuesOfContinuousStatesChanged,
    fmi3Boolean* nextEventTimeDefined,
    fmi3Float64* nextEventTime) {
    return fmi3UpdateDiscreteStates_(instance, discreteStatesNeedUpdate, terminateSimulation,
        nominalsOfContinuousStatesChanged, valuesOfContinuousStatesChanged, nextEventTimeDefined, nextEventTime);
}


    /***************************************************
    Types for Functions for Model Exchange
    ****************************************************/

fmi3Status fmi3EnterContinuousTimeMode(fmi3Instance instance) {
    return fmiEnterContinuousTimeMode_(instance);
}


fmi3Status fmi3CompletedIntegratorStep(fmi3Instance instance,
    fmi3Boolean  noSetFMUStatePriorToCurrentPoint,
    fmi3Boolean* enterEventMode,
    fmi3Boolean* terminateSimulation) {
    return fmiCompletedIntegratorStep_(instance, noSetFMUStatePriorToCurrentPoint, enterEventMode, terminateSimulation);
}


fmi3Status fmi3SetTime(fmi3Instance instance, fmi3Float64 time) {
    return fmiSetTime_(instance, time);
}

fmi3Status fmi3SetContinuousStates(fmi3Instance instance,
    const fmi3Float64 continuousStates[],
    size_t nContinuousStates) {
    return fmiSetContinuousStates_(instance, continuousStates, nContinuousStates);
}

fmi3Status fmi3GetContinuousStateDerivatives(fmi3Instance instance,
    fmi3Float64 derivatives[],
    size_t nContinuousStates) {
    return fmiGetDerivatives_(instance, derivatives, nContinuousStates);
}


fmi3Status fmi3GetEventIndicators(fmi3Instance instance,
    fmi3Float64 eventIndicators[],
    size_t nEventIndicators) {
    return fmiGetEventIndicators_(instance, eventIndicators, nEventIndicators);
}


fmi3Status fmi3GetContinuousStates(fmi3Instance instance,
    fmi3Float64 continuousStates[],
    size_t nContinuousStates) {
    return fmiGetContinuousStates_(instance, continuousStates, nContinuousStates);
}


fmi3Status fmi3GetNominalsOfContinuousStates(fmi3Instance instance,
    fmi3Float64 nominals[],
    size_t nContinuousStates) {
    return fmi3GetNominalsOfContinuousStates_(instance, nominals, nContinuousStates);
}

fmi3Status fmi3GetNumberOfEventIndicators(fmi3Instance instance,
    size_t* nEventIndicators) {
    return fmi3GetNumberOfEventIndicators_(instance, nEventIndicators);
}

fmi3Status fmi3GetNumberOfContinuousStates(fmi3Instance instance,
    size_t* nContinuousStates) {
    return fmi3GetNumberOfContinuousStates_(instance, nContinuousStates);
}
    /***************************************************
    Types for Functions for Co-Simulation
    ****************************************************/

    /* Simulating the FMU */


fmi3Status fmi3EnterStepMode(fmi3Instance instance) {
    return fmi3EnterStepMode_(instance);
}

fmi3Status fmi3GetOutputDerivatives(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int32 orders[],
    fmi3Float64 values[],
    size_t nValues) {
    return fmi3GetOutputDerivatives_(instance, valueReferences, nValueReferences, orders, values, nValues);
}


fmi3Status fmi3DoStep(fmi3Instance instance,
    fmi3Float64 currentCommunicationPoint,
    fmi3Float64 communicationStepSize,
    fmi3Boolean noSetFMUStatePriorToCurrentPoint,
    fmi3Boolean* eventEncountered,
    fmi3Boolean* terminateSimulation,
    fmi3Boolean* earlyReturn,
    fmi3Float64* lastSuccessfulTime) {
    return fmi3DoStep_(instance, currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint,
        eventEncountered, terminateSimulation, earlyReturn, lastSuccessfulTime);
}

fmi3Status fmi3ActivateModelPartition(fmi3Instance instance,
    fmi3ValueReference clockReference,
    fmi3Float64 activationTime) {
    return fmi3ActivateModelPartition_(instance, clockReference, activationTime);
}


#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

