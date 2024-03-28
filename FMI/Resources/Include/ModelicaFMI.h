#pragma once

#include <stdio.h>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

typedef struct {

    const char* tempBinaryDir;
    const char* tempBinaryPath;
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
    const char* logFile,
    int copyPlatformBinary);

EXPORT void FMU_free(void* instance);

void* FMU_getBuffer(void* instance, size_t size);

#include "FMI.c"
#include "FMI2.c"
#include "FMI3.c"
#include "ModelicaFMI.c"
#include "ModelicaFMI2.c"
#include "ModelicaFMI3.c"
