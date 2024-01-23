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

    if (instance->userData) {

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
    const char* logFile) {

    char platformBinaryPath[2048] = "";

    FMIPlatformBinaryPath(unzipdir, modelIdentifier, fmiVersion, platformBinaryPath, 2048);

    FMIInstance* S = FMICreateInstance(instanceName, platformBinaryPath, logMessage, logFMICalls ? logFunctionCall : NULL);

    if (!S) {
        ModelicaFormatError("Failed to load platform binary %s.", platformBinaryPath);
    }

    S->userData = calloc(1, sizeof(FMU_UserData));

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
                S,                  // instance,
                instantiationToken, // instantiationToken,
                resourcePath,       // resourcePath,
                visible,            // visible,
                loggingOn           // loggingOn
            ) :
            FMI3InstantiateCoSimulation(
                S,                  // instance,
                instantiationToken, // instantiationToken,
                resourcePath,       // resourcePath,
                visible,            // visible,
                loggingOn,          // loggingOn,
                fmi3False,          // eventModeUsed,
                fmi3False,          // earlyReturnAllowed,
                NULL,               // requiredIntermediateVariables[],
                0,                  // nRequiredIntermediateVariables,
                NULL                // intermediateUpdate
            );
    }

    if (status > FMIOK) {
        ModelicaFormatError("Failed to instantiate FMU %s.", platformBinaryPath);
    }
	
	return S;
}

void FMU_free(void* instance) {

    // TODO: terminate

    FMIInstance* S = (FMIInstance*)instance;

    if (S->userData) {

        FMU_UserData* userData = (FMU_UserData*)S->userData;

        if (userData->valueBuffer) {
            free(userData->valueBuffer);
        }

        free(userData);
    }

    FMIFreeInstance(S);
}
