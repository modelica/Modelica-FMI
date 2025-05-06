#pragma once

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

/***************************************************
Common Functions
****************************************************/

EXPORT void FMU_FMI3EnterInitializationMode(
    FMUInstance* instance,
    int toleranceDefined,
    double tolerance,
    double startTime,
    int stopTimeDefined,
    double stopTime);

EXPORT void FMU_FMI3ExitInitializationMode(FMUInstance* instance);

EXPORT void FMU_FMI3EnterEventMode(FMUInstance* instance);

EXPORT void FMU_FMI3EnterConfigurationMode(FMUInstance* instance);

EXPORT void FMU_FMI3ExitConfigurationMode(FMUInstance* instance);

EXPORT void FMU_FMI3GetFloat32(FMUInstance* instance, int valueReference, double values[], int nValues);

EXPORT void FMU_FMI3GetFloat64(FMUInstance* instance, int valueReference, double values[], int nValues);

EXPORT void FMU_FMI3GetInt32(FMUInstance* instance, int valueReference, int values[], int nValues);

EXPORT void FMU_FMI3GetInt64(FMUInstance* instance, int valueReference, int values[], int nValues);

EXPORT void FMU_FMI3GetUInt64(FMUInstance* instance, int valueReference, int values[], int nValues);

EXPORT void FMU_FMI3GetBoolean(FMUInstance* instance, int valueReference, int values[], int nValues);

EXPORT void FMU_FMI3SetFloat32(FMUInstance* instance, const int valueReferences[], int nValueReferences, const double values[], int nValues);

EXPORT void FMU_FMI3SetFloat64(FMUInstance* instance, const int valueReferences[], int nValueReferences, const double values[], int nValues);

EXPORT void FMU_FMI3SetInt32(FMUInstance* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues);

EXPORT void FMU_FMI3SetInt64(FMUInstance* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues);

EXPORT void FMU_FMI3SetUInt64(FMUInstance* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues);

EXPORT void FMU_FMI3SetBoolean(FMUInstance* instance, const int valueReferences[], int nValueReferences, const int values[], int nValues);

EXPORT void FMU_FMI3SetString(FMUInstance* instance, const int valueReferences[], int nValueReferences, const char* values[], int nValues);

EXPORT void FMU_FMI3UpdateDiscreteStates(FMUInstance* instance, int* valuesOfContinuousStatesChanged, double* nextEventTime);

/***************************************************
Functions for Model Exchange
****************************************************/

EXPORT void FMU_FMI3EnterContinuousTimeMode(FMUInstance* instance);

EXPORT void FMU_FMI3SetTime(FMUInstance* instance, double time);

EXPORT void FMU_FMI3SetContinuousStates(FMUInstance* instance, const double continuousStates[], int nContinuousStates);

EXPORT void FMU_FMI3GetContinuousStateDerivatives(FMUInstance* instance, double derivatives[], int nContinuousStates);

EXPORT void FMU_FMI3GetEventIndicators(FMUInstance* instance, double eventIndicators[], int nEventIndicators);

EXPORT void FMU_FMI3GetContinuousStates(FMUInstance* instance, double continuousStates[], int nContinuousStates);

/***************************************************
Functions for Co-Simulation
****************************************************/

EXPORT void FMU_FMI3DoStep(FMUInstance* instance, double currentCommunicationPoint, double communicationStepSize);
