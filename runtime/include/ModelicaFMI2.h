#pragma once

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

/***************************************************
Common Functions
****************************************************/

EXPORT void FMU_FMI2GetReal(FMUInstance* instance, int vr, double* value);

EXPORT void FMU_FMI2GetInteger(FMUInstance* instance, int vr, int* value);

EXPORT void FMU_FMI2GetBoolean(FMUInstance* instance, int vr, int* value);

EXPORT void FMU_FMI2SetReal(FMUInstance* instance, const int vr[], int nvr, const double value[]);

EXPORT void FMU_FMI2SetInteger(FMUInstance* instance, const int vr[], int nvr, const int value[]);

EXPORT void FMU_FMI2SetBoolean(FMUInstance* instance, const int vr[], int nvr, const int value[]);

EXPORT void FMU_FMI2SetString(FMUInstance* instance, const int vr[], int nvr, const char* value[]);

EXPORT void FMU_FMI2SetupExperiment(FMUInstance* instance,
    int toleranceDefined,
    double tolerance,
    double startTime,
    int stopTimeDefined,
    double stopTime);

EXPORT void FMU_FMI2EnterInitializationMode(FMUInstance* instance);

EXPORT void FMU_FMI2ExitInitializationMode(FMUInstance* instance);

/***************************************************
Model Exchange
****************************************************/

EXPORT void FMU_FMI2EnterEventMode(FMUInstance* instance);

EXPORT void FMU_FMI2NewDiscreteStates(FMUInstance* instance, int* valuesOfContinuousStatesChanged, double* nextEventTime);

EXPORT void FMU_FMI2EnterContinuousTimeMode(FMUInstance* instance);

EXPORT void FMU_FMI2SetTime(FMUInstance* instance, double time);

EXPORT void FMU_FMI2SetContinuousStates(FMUInstance* instance, const double x[], int nx);

EXPORT void FMU_FMI2GetDerivatives(FMUInstance* instance, double derivatives[], int nx);

EXPORT void FMU_FMI2GetEventIndicators(FMUInstance* instance, double eventIndicators[], int ni);

EXPORT void FMU_FMI2GetContinuousStates(FMUInstance* instance, double x[], int nx);

/***************************************************
Co-Simulation
****************************************************/

EXPORT void FMU_FMI2DoStep(FMUInstance* instance,
    double currentCommunicationPoint,
    double communicationStepSize,
    int noSetFMUStatePriorToCurrentPoint);
