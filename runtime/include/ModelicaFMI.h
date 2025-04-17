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

    char* errorMessage;

    char** infoMessages;
    size_t infoMessagesSize;
    size_t infoMessagesCapacity;
    size_t nextInfoMessage;

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
    int copyPlatformBinary
);

EXPORT void FMU_free(void* instance);

EXPORT const char* FMU_getInfoMessage(void* instance);

EXPORT const char* FMU_getWarningMessage(void* instance);

//EXPORT size_t FMU_getErrorMessagesSize(void* instance);
//
//EXPORT const char** FMU_getErrorMessages(void* instance);

EXPORT const char* FMU_getErrorMessage(void* instance);

void* FMU_getBuffer(void* instance, size_t size);
