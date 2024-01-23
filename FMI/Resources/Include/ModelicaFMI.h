#pragma once

#include "ModelicaFMI2.h"

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

typedef struct {

    FILE* logFile;
    char* valueBuffer;
    size_t valueBufferSize;

} FMU_UserData;

EXPORT void* FMU_load(
    const char* unzipdir,
    int fmiVersion,
    const char* modelIdentifier,
    const char* instanceName,
    int interfaceType,
    const char* instantiationToken,
    int visible,
    int loggingOn,
    int logFMICalls,
    int logToFile,
    const char* logFile);

EXPORT void FMU_free(void* instance);

#include "FMI.c"
#include "FMI2.c"
#include "FMI3.c"
#include "ModelicaFMI.c"
#include "ModelicaFMI2.c"
#include "ModelicaFMI3.c"
