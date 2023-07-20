#pragma once

#include <stdio.h>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

typedef struct {

    char** messages;
    size_t size;
    size_t capacity;
    size_t next;

} FMUMessageList;

typedef struct FMIInstance_ FMIInstance;

typedef struct {

    FMIInstance* instance;

    const char* tempBinaryDir;
    const char* tempBinaryPath;
    FILE* logFile;
    char* valueBuffer;
    size_t valueBufferSize;

    FMUMessageList* infoMessages;
    FMUMessageList* warningMessages;
    char* errorMessage;

} FMUInstance;

void FMUAppendMessage(FMUMessageList* list, const char* message);

const char* FMUGetMessage(FMUMessageList* list);

void FMUFreeMessageList(FMUMessageList* list);

void FMULogInfo(FMUInstance* userData, const char* message);

void FMULogWarning(FMUInstance* userData, const char* message);

void FMULogError(FMUInstance* userData, const char* message, ...);

void* FMUGetBuffer(FMUInstance* instance, size_t size);

EXPORT FMUInstance* FMU_Create();

EXPORT void FMU_Free(FMUInstance* instance);

EXPORT void FMU_Load(
    FMUInstance* instance,
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

EXPORT const char* FMU_getInfoMessage(FMUInstance* instance);

EXPORT const char* FMU_getWarningMessage(FMUInstance* instance);

EXPORT const char* FMU_getErrorMessage(FMUInstance* instance);
