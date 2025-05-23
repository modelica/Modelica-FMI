#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#include <unistd.h>
#endif

#include "ModelicaFMI.h"
#include "FMI2.h"
#include "FMI3.h"


static void logMessage(FMIInstance* instance, FMIStatus status, const char* category, const char* message) {

    if (!instance || !instance->userData) {
        return;
    }

    FMUInstance* externalObject = (FMUInstance*)instance->userData;

    switch (status) {
    case FMIOK:
        FMULogInfo(externalObject, message);
        break;
    case FMIWarning:
        FMULogWarning(externalObject, message);
        break;
    case FMIDiscard:
    case FMIError:
    case FMIFatal:
    case FMIPending:
        FMULogError(externalObject, message);
        break;
    }
}

void FMUAppendMessage(FMUMessageList* list, const char* message) {

    if (!list || !message) {
        return;
    }

    if (list->capacity < list->size + 1) {
        list->messages = realloc(list->messages, (++(list->capacity)) * sizeof(char*));
    }

    if (list->messages) {
        (list->messages)[(list->size)++] = strdup(message);
    }
}

const char* FMUGetMessage(FMUMessageList* list) {

    if (!list) {
        return "";
    }

    if (list->next < list->size) {
        return list->messages[list->next++];
    }

    for (size_t i = 0; i < list->size; i++) {
        free(list->messages[i]);
        list->messages[i] = NULL;
    }

    list->size = 0;
    list->next = 0;

    return "";
}

void FMUFreeMessageList(FMUMessageList* list) {
    
    if (list) {

        if (list->messages) {

            for (size_t i = 0; i < list->capacity; i++) {
                free(list->messages[i]);
            }

            free(list->messages);
        }

        free(list);
    }
}

void FMULogInfo(FMUInstance* instance, const char* message) {
    FMUAppendMessage(instance->infoMessages, message);
}

void FMULogWarning(FMUInstance* instance, const char* message) {
    FMUAppendMessage(instance->warningMessages, message);
}

void FMULogError(FMUInstance* instance, const char* format, ...) {

    va_list args1, args2;

    va_start(args1, format);
    va_copy(args2, args1);

    const size_t origMessageLength = instance->errorMessage ? strlen(instance->errorMessage) : 0;

    const size_t newMessageLength = origMessageLength + sizeof('\n') + vsnprintf(NULL, 0, format, args1);

    instance->errorMessage = realloc(instance->errorMessage, newMessageLength);

    if (instance->errorMessage) {
        vsnprintf(&instance->errorMessage[origMessageLength], newMessageLength, format, args2);
    }

    va_end(args1);
    va_end(args2);
}

static void logFunctionCall(FMIInstance* instance, FMIStatus status, const char* call) {

    const char* suffix;

    switch (status) {
    case FMIOK:
        suffix = " -> OK\n";
        break;
    case FMIWarning:
        suffix = " -> Warning\n";
        break;
    case FMIDiscard:
        suffix = " -> Discard\n";
        break;
    case FMIError:
        suffix = " -> Error\n";
        break;
    case FMIFatal:
        suffix = " -> Fatal\n";
        break;
    case FMIPending:
        suffix = " -> Pending\n";
        break;
    default:
        suffix = " -> Illegal return code\n";
        break;
    }

    FMUInstance* object = (FMUInstance*)instance->userData;

    if (object && object->logFile) {

        FILE* logFile = object->logFile;
        
        if (logFile) {
            fprintf(logFile, "[%s] ", instance->name);
            fputs(call, logFile);
            fputs(suffix, logFile);
        }

    } else {

        switch (status) {
        case FMIOK:
            FMULogInfo(object, call);
            break;
        case FMIWarning:
            FMULogWarning(object, call);
            break;
        case FMIDiscard:
        case FMIError:
        case FMIFatal:
        case FMIPending:
            FMULogError(object, call);
            break;
        }

    }
}

FMUInstance* FMU_Create() {

    FMUInstance* instance = (FMUInstance*)calloc(1, sizeof(FMUInstance));

    instance->infoMessages = calloc(1, sizeof(FMUMessageList));
    instance->warningMessages = calloc(1, sizeof(FMUMessageList));

    return instance;
}

void FMU_Free(FMUInstance* instance) {

    /* TODO: terminate? */

    if (instance->instance->fmiMajorVersion == FMIMajorVersion2) {
        FMI2FreeInstance(instance->instance);
    } else if (instance->instance->fmiMajorVersion == FMIMajorVersion3) {
        FMI3FreeInstance(instance->instance);
    } 

    FMIFreeInstance(instance->instance);

    if (instance->tempBinaryPath) {

       char command[4096];
#ifdef WIN32
        snprintf(command, 4096, "rmdir /s /q \"%s\"", instance->tempBinaryDir);
#else
        snprintf(command, 4096, "rm -rf \"%s\"", instance->tempBinaryDir);
#endif
        if (system(command)) {
            FMULogError(instance, "Failed to remove temporary directory.");
        }

        free((void*)instance->tempBinaryPath);
    }

    FMUFreeMessageList(instance->infoMessages);
    FMUFreeMessageList(instance->warningMessages);

    if (instance->valueBuffer) {
        free(instance->valueBuffer);
    }

    free(instance);
}

void FMU_Load(
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
    int copyPlatformBinary) {

    char platformBinaryPath[2048] = "";

    FMIPlatformBinaryPath(unzipdir, modelIdentifier, (FMIMajorVersion)fmiVersion, platformBinaryPath, 2048);

#if defined(_WIN32)
    const char* separator = "\\";
    const char* extension = ".dll";
    const char* copyCommand = "copy";
#elif defined(__APPLE__)
    const char separator = '/';
    const char* extension = ".dylib";
    const char* copyCommand = "cp";
#else
    const char separator = '/';
    const char* extension = ".so";
    const char* copyCommand = "cp";
#endif

    FMIInstance* S = FMICreateInstance(
        instanceName,
        logMessage,
        logFMICalls ? ((FMILogFunctionCall*)logFunctionCall) : NULL
    );

    S->userData = instance;

    char command[4096];

    if (copyPlatformBinary) {
#ifdef WIN32
        instance->tempBinaryDir = _tempnam(NULL, NULL);
#else
        instance->tempBinaryDir = (char*)calloc(1, 2048);

        sprintf(instance->tempBinaryDir, "/tmp/fmusim.XXXXXX");

        mkdtemp(instance->tempBinaryDir);
#endif
        if (!instance->tempBinaryDir) {
            FMULogError(instance, "Failed to create temporary directory path.");
        }

        snprintf(command, 4096, "mkdir \"%s\"", instance->tempBinaryDir);

        if (system(command)) {
            FMULogError(instance, "Failed to create temporary directory. Command: %s", command);
        }

        instance->tempBinaryPath = (char*)calloc(1, strlen(instance->tempBinaryDir) + strlen(separator) + strlen(modelIdentifier) + strlen(extension) + 1);
        
        sprintf((char*)instance->tempBinaryPath, "%s%s%s%s", instance->tempBinaryDir, separator, modelIdentifier, extension);
        
        snprintf(command, 4096, "%s \"%s\" \"%s\"", copyCommand, platformBinaryPath, instance->tempBinaryDir);

        if (system(command)) {
            FMULogError(instance, "Failed to copy platform binary. Command: %s", command);
        }
    }

    if (!S) {
        FMULogError(instance, "Failed to create instance.");
        return;
    }

    FMILoadPlatformBinary(S, copyPlatformBinary ? instance->tempBinaryPath : platformBinaryPath);

    if (!S->libraryHandle) {
        FMULogError(instance, "Failed to load platform binary %s.", platformBinaryPath);
        return;
    }

    instance->instance = S;

    if (logToFile && logFile) {
        instance->logFile = fopen(logFile, "w");
    }

    char resourcePath[4096] = "";

    strcpy(resourcePath, unzipdir);

#ifdef _WIN32
    strcat(resourcePath, "\\resources\\");
    _fullpath(resourcePath, resourcePath, sizeof(resourcePath));
#else
    strcat(resourcePath, "/resources/");
    realpath(resourcePath, resourcePath);
#endif

    FMIStatus status = FMIFatal;

    if (fmiVersion == FMIMajorVersion2) {
        char resourceURI[4096] = "";
        FMIPathToURI(resourcePath, resourceURI, 4096);
        status = FMI2Instantiate(S, resourceURI, (fmi2Type)interfaceType, instantiationToken, visible, loggingOn);
    } else {
        status = interfaceType == FMIModelExchange ?
            FMI3InstantiateModelExchange(
                S,                  /* instance, */
                instantiationToken, /* instantiationToken, */
                resourcePath,       /* resourcePath, */
                visible,            /* visible, */
                loggingOn           /* loggingOn */
            ) :
            FMI3InstantiateCoSimulation(
                S,                  /* instance, */
                instantiationToken, /* instantiationToken, */
                resourcePath,       /* resourcePath, */
                visible,            /* visible, */
                loggingOn,          /* loggingOn, */
                fmi3False,          /* eventModeUsed, */
                fmi3False,          /* earlyReturnAllowed, */
                NULL,               /* requiredIntermediateVariables[], */
                0,                  /* nRequiredIntermediateVariables, */
                NULL                /* intermediateUpdate */
            );
    }

    if (status > FMIOK) {
        FMULogError(instance, "Failed to instantiate FMU %s.", platformBinaryPath);
    }
}

const char* FMU_getInfoMessage(FMUInstance* instance) {

    if (!instance) {
        return "";
    }    
    
    return FMUGetMessage(instance->infoMessages);
}

const char* FMU_getWarningMessage(FMUInstance* instance) {

    if (!instance) {
        return "";
    }

    return FMUGetMessage(instance->warningMessages);
}

const char* FMU_getErrorMessage(FMUInstance* instance) {
    
    if (instance->errorMessage) {
        return instance->errorMessage;
    }

    return "";
}

void* FMUGetBuffer(FMUInstance* instance, size_t size) {

    if (instance->valueBufferSize < size) {

        void* temp = realloc(instance->valueBuffer, size);
        
        if (!temp) {
            FMULogError(instance, "Failed to allocate memory for value buffer.");
        }
        
        instance->valueBuffer = (char*)temp;
        instance->valueBufferSize = size;
    }

    return instance->valueBuffer;
}
