#include <math.h>
#include <limits.h>
#include <float.h>

#include "ModelicaUtilities.h"
#include "ModelicaFMI3.h"
#include "FMI3.h"


#define CALL(f) do { if (f > FMIWarning) { ModelicaFormatError("The FMU reported an error."); } } while (0)

/***************************************************
Common Functions
****************************************************/

void FMU_FMI3EnterInitializationMode(
    void* instance,
    int toleranceDefined,
    double tolerance,
    double startTime,
    int stopTimeDefined,
    double stopTime) {

    CALL(FMI3EnterInitializationMode(
        (FMIInstance*)instance,
        toleranceDefined,
        tolerance,
        startTime,
        stopTimeDefined,
        stopTime
    ));
}

void FMU_FMI3ExitInitializationMode(void* instance) {
    CALL(FMI3ExitInitializationMode((FMIInstance*)instance));
}

void FMU_FMI3EnterEventMode(void* instance) {
    CALL(FMI3EnterEventMode((FMIInstance*)instance));
}

void FMU_FMI3EnterConfigurationMode(void* instance) {
    CALL(FMI3EnterConfigurationMode((FMIInstance*)instance));
}

void FMU_FMI3ExitConfigurationMode(void* instance) {
    CALL(FMI3ExitConfigurationMode((FMIInstance*)instance));
}

void FMU_FMI3GetFloat32(void* instance, int valueReference, double values[], int nValues) {

    size_t i;

    const fmi3ValueReference vr = valueReference;

    fmi3Float32* buffer = (fmi3Float32*)FMU_getBuffer((FMIInstance*)instance, nValues * sizeof(fmi3Float32));

    CALL(FMI3GetFloat32((FMIInstance*)instance, &vr, 1, buffer, nValues));

    for (i = 0; i < nValues; i++) {
        values[i] = buffer[i];
    }
}

void FMU_FMI3GetFloat64(void* instance, int valueReference, double values[], int nValues) {
    const fmi3ValueReference vr = valueReference;
    CALL(FMI3GetFloat64((FMIInstance*)instance, &vr, 1, values, nValues));
}

void FMU_FMI3GetInt32(void* instance, int valueReference, int values[], int nValues) {
    const fmi3ValueReference vr = valueReference;
    CALL(FMI3GetInt32((FMIInstance*)instance, &vr, 1, values, nValues));
}

void FMU_FMI3GetInt64(void* instance, int valueReference, int values[], int nValues) {

    size_t i;

    const fmi3ValueReference vr = valueReference;

    fmi3Int64* buffer = (fmi3Int64*)FMU_getBuffer((FMIInstance*)instance, nValues * sizeof(fmi3Int64));

    CALL(FMI3GetInt64((FMIInstance*)instance, &vr, 1, buffer, nValues));

    for (i = 0; i < nValues; i++) {
        const fmi3Int64 value = buffer[i];
        if (value < INT_MIN || value > INT_MAX) {
            ModelicaFormatError("Value exceeds allowed range of Integer.");
        }
        values[i] = (int)value;
    }
}

void FMU_FMI3GetUInt64(void* instance, int valueReference, int values[], int nValues) {

    size_t i;

    const fmi3ValueReference vr = valueReference;

    fmi3UInt64* buffer = (fmi3UInt64*)FMU_getBuffer((FMIInstance*)instance, nValues * sizeof(fmi3UInt64));

    CALL(FMI3GetUInt64((FMIInstance*)instance, &vr, 1, buffer, nValues));

    for (i = 0; i < nValues; i++) {
        const fmi3UInt64 value = buffer[i];
        if (value > INT_MAX) {
            ModelicaFormatError("Value exceeds allowed range of Integer.");
        }
        values[i] = (int)value;
    }
}

void FMU_FMI3GetBoolean(void* instance, int valueReference, int values[], int nValues) {

    size_t i;

    const fmi3ValueReference vr = valueReference;

    fmi3Boolean* buffer = (fmi3Boolean*)FMU_getBuffer((FMIInstance*)instance, nValues * sizeof(fmi3Boolean));

    CALL(FMI3GetBoolean((FMIInstance*)instance, &vr, 1, (fmi3Boolean*)values, nValues));

    for (i = 0; i < nValues; i++) {
        values[i] = buffer[i];
    }
}

void FMU_FMI3SetFloat32(void* instance, const int valueReferences[], int nValueReferences, const double values[], int nValues) {

    size_t i;

    fmi3Float32* buffer = (fmi3Float32*)FMU_getBuffer((FMIInstance*)instance, nValues * sizeof(fmi3Float32));

    for (i = 0; i < nValues; i++) {

        const double value = values[i];

        if (value < -FLT_MAX || value > FLT_MAX) {
            ModelicaFormatError("Value exceeds allowed range of fmi3Float32.");
        }

        buffer[i] = (fmi3Float32)value;
    }

    CALL(FMI3SetFloat32((FMIInstance*)instance, valueReferences, nValueReferences, buffer, nValues));
}

void FMU_FMI3SetFloat64(void* instance, const int valueReferences[], int nValueReferences, const double values[], int nValues) {
    CALL(FMI3SetFloat64((FMIInstance*)instance, valueReferences, nValueReferences, values, nValues));
}

void FMU_FMI3SetInt32(void* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues) {
    CALL(FMI3SetInt32((FMIInstance*)instance, valueReferences, nValueReferences, values, nValues));
}

void FMU_FMI3SetInt64(void* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues) {

    size_t i;

    fmi3Int64* buffer = (fmi3Int64*)FMU_getBuffer((FMIInstance*)instance, nValues * sizeof(fmi3Int64));

    for (i = 0; i < nValues; i++) {
        buffer[i] = values[i];
    }

    CALL(FMI3SetInt64((FMIInstance*)instance, valueReferences, nValueReferences, buffer, nValues));
}

void FMU_FMI3SetUInt64(void* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues) {

    size_t i;

    fmi3UInt64* buffer = (fmi3UInt64*)FMU_getBuffer((FMIInstance*)instance, nValues * sizeof(fmi3UInt64));

    for (i = 0; i < nValues; i++) {
        buffer[i] = values[i];
    }

    CALL(FMI3SetUInt64((FMIInstance*)instance, valueReferences, nValueReferences, buffer, nValues));
}

void FMU_FMI3SetBoolean(void* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues) {
    CALL(FMI3SetBoolean((FMIInstance*)instance, valueReferences, nValueReferences, (fmi3Boolean*)values, nValues));
}

void FMU_FMI3SetString(void* instance, const int valueReferences[], int nValueReferences, const char* values[], int nValues) {
    CALL(FMI3SetString((FMIInstance*)instance, valueReferences, nValueReferences, (fmi3String*)values, nValues));
}

void FMU_FMI3UpdateDiscreteStates(void* instance, int* valuesOfContinuousStatesChanged, double* nextEventTime) {

    fmi3Boolean _discreteStatesNeedUpdate;
    fmi3Boolean _terminateSimulation;
    fmi3Boolean _nominalsOfContinuousStatesChanged;
    fmi3Boolean _valuesOfContinuousStatesChanged;
    fmi3Boolean _nextEventTimeDefined;
    fmi3Float64 _nextEventTime;

    do {
        CALL(FMI3UpdateDiscreteStates(
            (FMIInstance*)instance,
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

void FMU_FMI3EnterContinuousTimeMode(void* instance) {
    CALL(FMI3EnterContinuousTimeMode((FMIInstance*)instance));
}

void FMU_FMI3SetTime(void* instance, double time) {
    CALL(FMI3SetTime((FMIInstance*)instance, time));
}

void FMU_FMI3SetContinuousStates(void* instance, const double continuousStates[], int nContinuousStates) {
    if (nContinuousStates > 0) CALL(FMI3SetContinuousStates((FMIInstance*)instance, continuousStates, nContinuousStates));
}

void FMU_FMI3GetContinuousStateDerivatives(void* instance, double derivatives[], int nContinuousStates) {
    if (nContinuousStates > 0) CALL(FMI3GetContinuousStateDerivatives((FMIInstance*)instance, derivatives, nContinuousStates));
}

void FMU_FMI3GetEventIndicators(void* instance, double eventIndicators[], int nEventIndicators) {
    if (nEventIndicators > 0) CALL(FMI3GetEventIndicators((FMIInstance*)instance, eventIndicators, nEventIndicators));
}

void FMU_FMI3GetContinuousStates(void* instance, double continuousStates[], int nContinuousStates) {
    if (nContinuousStates > 0) CALL(FMI3GetContinuousStates((FMIInstance*)instance, continuousStates, nContinuousStates));
}

/***************************************************
Functions for Co-Simulation
****************************************************/

void FMU_FMI3DoStep(
    void* instance,
    double currentCommunicationPoint,
    double communicationStepSize) {

    fmi3Boolean eventHandlingNeeded;
    fmi3Boolean terminateSimulation;
    fmi3Boolean earlyReturn;
    fmi3Float64 lastSuccessfulTime;

    CALL(FMI3DoStep(
        (FMIInstance*)instance,
        currentCommunicationPoint,
        communicationStepSize,
        fmi3True,
        &eventHandlingNeeded,
        &terminateSimulation,
        &earlyReturn,
        &lastSuccessfulTime
    ));
}
