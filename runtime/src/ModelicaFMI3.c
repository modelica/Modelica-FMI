#include <math.h>
#include <limits.h>
#include <float.h>

#include "ModelicaFMI.h"
#include "ModelicaFMI3.h"
#include "FMI3.h"


// TODO: check instance != NULL
#define CALL(f) do { if (f > FMIWarning) { /* FMUAppendMessage( ((FMU_UserData*) ( (instance->instance)->userData) )->infoMessages, "The FMU reported an error."); */ } } while (0)


/***************************************************
Common Functions
****************************************************/

void FMU_FMI3EnterInitializationMode(
    FMUInstance* instance,
    int toleranceDefined,
    double tolerance,
    double startTime,
    int stopTimeDefined,
    double stopTime) {

    CALL(FMI3EnterInitializationMode(
        instance->instance,
        toleranceDefined,
        tolerance,
        startTime,
        stopTimeDefined,
        stopTime
    ));
}

void FMU_FMI3ExitInitializationMode(FMUInstance* instance) {
    CALL(FMI3ExitInitializationMode(instance->instance));
}

void FMU_FMI3EnterEventMode(FMUInstance* instance) {
    CALL(FMI3EnterEventMode(instance->instance));
}

void FMU_FMI3EnterConfigurationMode(FMUInstance* instance) {
    CALL(FMI3EnterConfigurationMode(instance->instance));
}

void FMU_FMI3ExitConfigurationMode(FMUInstance* instance) {
    CALL(FMI3ExitConfigurationMode(instance->instance));
}

void FMU_FMI3GetFloat32(FMUInstance* instance, int valueReference, double values[], int nValues) {

    size_t i;

    const fmi3ValueReference vr = valueReference;

    fmi3Float32* buffer = (fmi3Float32*)FMUGetBuffer(instance, nValues * sizeof(fmi3Float32));

    CALL(FMI3GetFloat32(instance->instance, &vr, 1, buffer, nValues));

    for (i = 0; i < nValues; i++) {
        values[i] = buffer[i];
    }
}

void FMU_FMI3GetFloat64(FMUInstance* instance, int valueReference, double values[], int nValues) {
    const fmi3ValueReference vr = valueReference;
    CALL(FMI3GetFloat64(instance->instance, &vr, 1, values, nValues));
}

void FMU_FMI3GetInt32(FMUInstance* instance, int valueReference, int values[], int nValues) {
    const fmi3ValueReference vr = valueReference;
    CALL(FMI3GetInt32(instance->instance, &vr, 1, values, nValues));
}

void FMU_FMI3GetInt64(FMUInstance* instance, int valueReference, int values[], int nValues) {

    const fmi3ValueReference vr = valueReference;

    fmi3Int64* buffer = (fmi3Int64*)FMUGetBuffer(instance, nValues * sizeof(fmi3Int64));

    CALL(FMI3GetInt64(instance->instance, &vr, 1, buffer, nValues));

    for (size_t i = 0; i < nValues; i++) {
        const fmi3Int64 value = buffer[i];
        if (value < INT_MIN || value > INT_MAX) {
            FMULogError(instance, "Value exceeds allowed range of Integer.");
        }
        values[i] = (int)value;
    }
}

void FMU_FMI3GetUInt64(FMUInstance* instance, int valueReference, int values[], int nValues) {

    const fmi3ValueReference vr = valueReference;

    fmi3UInt64* buffer = (fmi3UInt64*)FMUGetBuffer(instance, nValues * sizeof(fmi3UInt64));

    CALL(FMI3GetUInt64(instance->instance, &vr, 1, buffer, nValues));

    for (size_t i = 0; i < nValues; i++) {
        const fmi3UInt64 value = buffer[i];
        if (value > INT_MAX) {
            FMULogError(instance, "Value exceeds allowed range of Integer.");
        }
        values[i] = (int)value;
    }
}

void FMU_FMI3GetBoolean(FMUInstance* instance, int valueReference, int values[], int nValues) {

    size_t i;

    const fmi3ValueReference vr = valueReference;

    fmi3Boolean* buffer = (fmi3Boolean*)FMUGetBuffer(instance, nValues * sizeof(fmi3Boolean));

    CALL(FMI3GetBoolean(instance->instance, &vr, 1, (fmi3Boolean*)values, nValues));

    for (i = 0; i < nValues; i++) {
        values[i] = buffer[i];
    }
}

void FMU_FMI3SetFloat32(FMUInstance* instance, const int valueReferences[], int nValueReferences, const double values[], int nValues) {

    fmi3Float32* buffer = (fmi3Float32*)FMUGetBuffer(instance, nValues * sizeof(fmi3Float32));

    for (size_t i = 0; i < nValues; i++) {

        const double value = values[i];

        if (value < -FLT_MAX || value > FLT_MAX) {
            FMULogError(instance, "Value exceeds allowed range of fmi3Float32.");
        }

        buffer[i] = (fmi3Float32)value;
    }

    CALL(FMI3SetFloat32(instance->instance, valueReferences, nValueReferences, buffer, nValues));
}

void FMU_FMI3SetFloat64(FMUInstance* instance, const int valueReferences[], int nValueReferences, const double values[], int nValues) {
    CALL(FMI3SetFloat64(instance->instance, valueReferences, nValueReferences, values, nValues));
}

void FMU_FMI3SetInt32(FMUInstance* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues) {
    CALL(FMI3SetInt32(instance->instance, valueReferences, nValueReferences, values, nValues));
}

void FMU_FMI3SetInt64(FMUInstance* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues) {

    size_t i;

    fmi3Int64* buffer = (fmi3Int64*)FMUGetBuffer(instance, nValues * sizeof(fmi3Int64));

    for (i = 0; i < nValues; i++) {
        buffer[i] = values[i];
    }

    CALL(FMI3SetInt64(instance->instance, valueReferences, nValueReferences, buffer, nValues));
}

void FMU_FMI3SetUInt64(FMUInstance* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues) {

    size_t i;

    fmi3UInt64* buffer = (fmi3UInt64*)FMUGetBuffer(instance, nValues * sizeof(fmi3UInt64));

    for (i = 0; i < nValues; i++) {
        buffer[i] = values[i];
    }

    CALL(FMI3SetUInt64(instance->instance, valueReferences, nValueReferences, buffer, nValues));
}

void FMU_FMI3SetBoolean(FMUInstance* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues) {
    CALL(FMI3SetBoolean(instance->instance, valueReferences, nValueReferences, (fmi3Boolean*)values, nValues));
}

void FMU_FMI3SetString(FMUInstance* instance, const int valueReferences[], int nValueReferences, const char* values[], int nValues) {
    CALL(FMI3SetString(instance->instance, valueReferences, nValueReferences, (fmi3String*)values, nValues));
}

void FMU_FMI3UpdateDiscreteStates(FMUInstance* instance, int* valuesOfContinuousStatesChanged, double* nextEventTime) {

    fmi3Boolean _discreteStatesNeedUpdate;
    fmi3Boolean _terminateSimulation;
    fmi3Boolean _nominalsOfContinuousStatesChanged;
    fmi3Boolean _valuesOfContinuousStatesChanged;
    fmi3Boolean _nextEventTimeDefined;
    fmi3Float64 _nextEventTime;

    do {
        CALL(FMI3UpdateDiscreteStates(
            instance->instance,
            &_discreteStatesNeedUpdate,
            &_terminateSimulation,
            &_nominalsOfContinuousStatesChanged,
            &_valuesOfContinuousStatesChanged,
            &_nextEventTimeDefined,
            &_nextEventTime
        ));
    } while (_discreteStatesNeedUpdate);

    *valuesOfContinuousStatesChanged = _valuesOfContinuousStatesChanged;
    *nextEventTime = _nextEventTimeDefined ? _nextEventTime : HUGE_VAL;
}

/***************************************************
Functions for Model Exchange
****************************************************/

void FMU_FMI3EnterContinuousTimeMode(FMUInstance* instance) {
    CALL(FMI3EnterContinuousTimeMode(instance->instance));
}

void FMU_FMI3SetTime(FMUInstance* instance, double time) {
    CALL(FMI3SetTime(instance->instance, time));
}

void FMU_FMI3SetContinuousStates(FMUInstance* instance, const double continuousStates[], int nContinuousStates) {
    if (nContinuousStates > 0) CALL(FMI3SetContinuousStates(instance->instance, continuousStates, nContinuousStates));
}

void FMU_FMI3GetContinuousStateDerivatives(FMUInstance* instance, double derivatives[], int nContinuousStates) {
    if (nContinuousStates > 0) CALL(FMI3GetContinuousStateDerivatives(instance->instance, derivatives, nContinuousStates));
}

void FMU_FMI3GetEventIndicators(FMUInstance* instance, double eventIndicators[], int nEventIndicators) {
    if (nEventIndicators > 0) CALL(FMI3GetEventIndicators(instance->instance, eventIndicators, nEventIndicators));
}

void FMU_FMI3GetContinuousStates(FMUInstance* instance, double continuousStates[], int nContinuousStates) {
    if (nContinuousStates > 0) CALL(FMI3GetContinuousStates(instance->instance, continuousStates, nContinuousStates));
}

/***************************************************
Functions for Co-Simulation
****************************************************/

void FMU_FMI3DoStep(
    FMUInstance* instance,
    double currentCommunicationPoint,
    double communicationStepSize) {

    fmi3Boolean eventHandlingNeeded;
    fmi3Boolean terminateSimulation;
    fmi3Boolean earlyReturn;
    fmi3Float64 lastSuccessfulTime;

    CALL(FMI3DoStep(
        instance->instance,
        currentCommunicationPoint,
        communicationStepSize,
        fmi3True,
        &eventHandlingNeeded,
        &terminateSimulation,
        &earlyReturn,
        &lastSuccessfulTime
    ));
}
