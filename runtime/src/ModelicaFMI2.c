#include <math.h>

#include "ModelicaFMI.h"
#include "ModelicaFMI2.h"
#include "FMI2.h"


// TODO: check instance != NULL
#define CALL(f) do { if (f > FMIWarning) { /* FMUAppendMessage( ((FMU_UserData*) ( ((FMIInstance*)instance)->userData) )->infoMessages, "The FMU reported an error."); */ } } while (0)


/***************************************************
Common Functions
****************************************************/

void FMU_FMI2GetReal(FMUInstance* instance, int vr, double* value) {
    CALL(FMI2GetReal(instance->instance, &vr, 1, value));
}

void FMU_FMI2GetInteger(FMUInstance* instance, int vr, int* value) {
    CALL(FMI2GetInteger(instance->instance, &vr, 1, value));
}

void FMU_FMI2GetBoolean(FMUInstance* instance, int vr, int* value) {
    CALL(FMI2GetBoolean(instance->instance, &vr, 1, value));
}

void FMU_FMI2SetReal(FMUInstance* instance, const int vr[], int nvr, const double value[]) {
    if (nvr > 0) CALL(FMI2SetReal(instance->instance, vr, nvr, value));
}

void FMU_FMI2SetInteger(FMUInstance* instance, const int vr[], int nvr, const int value[]) {
    if (nvr > 0) CALL(FMI2SetInteger(instance->instance, vr, nvr, value));
}

void FMU_FMI2SetBoolean(FMUInstance* instance, const int vr[], int nvr, const int value[]) {
    if (nvr > 0) CALL(FMI2SetBoolean(instance->instance, vr, nvr, value));
}

void FMU_FMI2SetString(FMUInstance* instance, const int vr[], int nvr, const char* value[]) {
    if (nvr > 0) CALL(FMI2SetString(instance->instance, vr, nvr, value));
}

void FMU_FMI2SetupExperiment(FMUInstance* instance,
    int toleranceDefined,
    double tolerance,
    double startTime,
    int stopTimeDefined,
    double stopTime) {
    CALL(FMI2SetupExperiment(instance->instance, toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime));
}

void FMU_FMI2EnterInitializationMode(FMUInstance* instance) {
    CALL(FMI2EnterInitializationMode(instance->instance));
}

void FMU_FMI2ExitInitializationMode(FMUInstance* instance) {
    CALL(FMI2ExitInitializationMode(instance->instance));
}

/***************************************************
Model Exchange
****************************************************/

void FMU_FMI2EnterEventMode(FMUInstance* instance) {
    CALL(FMI2EnterEventMode(instance->instance));
}

void FMU_FMI2NewDiscreteStates(FMUInstance* instance, int* valuesOfContinuousStatesChanged, double* nextEventTime) {

    fmi2EventInfo eventInfo;

    do {
        CALL(FMI2NewDiscreteStates(instance->instance, &eventInfo));
    } while (eventInfo.newDiscreteStatesNeeded);

    *nextEventTime = eventInfo.nextEventTimeDefined ? eventInfo.nextEventTime : HUGE_VAL;
    *valuesOfContinuousStatesChanged = eventInfo.valuesOfContinuousStatesChanged;
}

void FMU_FMI2EnterContinuousTimeMode(FMUInstance* instance) {
    CALL(FMI2EnterContinuousTimeMode(instance->instance));
}

void FMU_FMI2SetTime(FMUInstance* instance, double time) {
    CALL(FMI2SetTime(instance->instance, time));
}

void FMU_FMI2SetContinuousStates(FMUInstance* instance, const double x[], int nx) {
    if (nx > 0) CALL(FMI2SetContinuousStates(instance->instance, x, nx));
}

void FMU_FMI2GetDerivatives(FMUInstance* instance, double derivatives[], int nx) {
    if (nx > 0) CALL(FMI2GetDerivatives(instance->instance, derivatives, nx));
}

void FMU_FMI2GetEventIndicators(FMUInstance* instance, double eventIndicators[], int ni) {
    if (ni > 0) CALL(FMI2GetEventIndicators(instance->instance, eventIndicators, ni));
}

void FMU_FMI2GetContinuousStates(FMUInstance* instance, double x[], int nx) {
    if (nx > 0) CALL(FMI2GetContinuousStates(instance->instance, x, nx));
}

/***************************************************
Co-Simulation
****************************************************/

void FMU_FMI2DoStep(FMUInstance* instance,
    double currentCommunicationPoint,
    double communicationStepSize,
    int noSetFMUStatePriorToCurrentPoint) {
    CALL(FMI2DoStep(instance->instance, currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint));
}
