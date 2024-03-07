#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ModelicaFMI.h"
#include "ModelicaUtilities.h"
#include "FMI2.h"
#include "FMI3.h"


static void logMessage(FMIInstance* instance, FMIStatus status, const char* category, const char* message) {
    ModelicaFormatMessage("%s\n", message);
}

static void logFunctionCall(FMIInstance* instance, FMIStatus status, const char* message, ...) {

    va_list args;
    va_start(args, message);

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

    if (instance->userData && ((FMU_UserData*)instance->userData)->logFile) {

        FILE* logFile = ((FMU_UserData*)instance->userData)->logFile;
        
        if (logFile) {
            fprintf(logFile, "[%s] ", instance->name);
            vfprintf(logFile, message, args);
            fprintf(logFile, suffix);
        }

    } else {
        ModelicaFormatMessage("[%s] ", instance->name);
        ModelicaVFormatMessage(message, args);
        ModelicaFormatMessage(suffix);
    }
    
    va_end(args);
}

void* FMU_load(
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

    FMIPlatformBinaryPath(unzipdir, modelIdentifier, (FMIVersion)fmiVersion, platformBinaryPath, 2048);

    FMU_UserData* userData = (FMU_UserData*)calloc(1, sizeof(FMU_UserData));

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

    char command[4096];

    if (copyPlatformBinary) {
#ifdef WIN32
        userData->tempBinaryDir = _tempnam(NULL, NULL);
#else
        userData->tempBinaryDir = (char*)calloc(1, 2048);

        sprintf(userData->tempBinaryDir, "/tmp/fmusim.XXXXXX");

        mkdtemp(userData->tempBinaryDir);
#endif
        if (!userData->tempBinaryDir) {
            ModelicaError("Failed to create temporary directory path.");
        }

        snprintf(command, 4096, "mkdir \"%s\"", userData->tempBinaryDir);

        if (system(command)) {
            ModelicaFormatError("Failed to create temporary directory. Command: %s", command);
        }

        userData->tempBinaryPath = (char*)calloc(1, strlen(userData->tempBinaryDir) + strlen(separator) + strlen(modelIdentifier) + strlen(extension) + 1);
        
        sprintf((char*)userData->tempBinaryPath, "%s%s%s%s", userData->tempBinaryDir, separator, modelIdentifier, extension);
        

        snprintf(command, 4096, "%s \"%s\" \"%s\"", copyCommand, platformBinaryPath, userData->tempBinaryDir);

        if (system(command)) {
            ModelicaFormatError("Failed to copy platform binary. Command: %s", command);
        }
    }

    FMIInstance* S = FMICreateInstance(
        instanceName, 
        copyPlatformBinary ? userData->tempBinaryPath : platformBinaryPath,
        logMessage, 
        logFMICalls ? ((FMILogFunctionCall*)logFunctionCall) : NULL
    );

    if (!S) {
        ModelicaFormatError("Failed to load platform binary %s.", platformBinaryPath);
    }

    S->userData = userData;

    if (logToFile && logFile) {

        FMU_UserData* userData = (FMU_UserData*)S->userData;

        userData->logFile = fopen(logFile, "w");
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

    if (fmiVersion == FMIVersion2) {
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
        ModelicaFormatError("Failed to instantiate FMU %s.", platformBinaryPath);
    }
	
	return S;
}

void FMU_free(void* instance) {

    /* TODO: terminate? */

    FMIInstance* S = (FMIInstance*)instance;

    FMU_UserData* userData = (FMU_UserData*)S->userData;

    char command[4096];

    FMIFreeInstance(S);

    if (userData) {

        if (userData->tempBinaryPath) {
#ifdef WIN32
            snprintf(command, 4096, "rmdir /s /q \"%s\"", userData->tempBinaryDir);
#else
            snprintf(command, 4096, "rm -rf \"%s\"", userData->tempBinaryDir);
#endif
            if (system(command)) {
                ModelicaFormatWarning("Failed to remove temporary directory. Command: %s", command);
            }

            free((void*)userData->tempBinaryPath);
        }

        if (userData->valueBuffer) {
            free(userData->valueBuffer);
        }

        free(userData);
    }
}

void* FMU_getBuffer(void* instance, size_t size) {

    FMU_UserData* userData = (FMU_UserData*)((FMIInstance*)instance)->userData;

    if (userData->valueBufferSize < size) {

        void* temp = realloc(userData->valueBuffer, size);
        
        if (!temp) {
            ModelicaFormatError("Failed to allocate memory for value buffer.");
        }
        
        userData->valueBuffer = (char*)temp;
        userData->valueBufferSize = size;
    }

    return userData->valueBuffer;
}
