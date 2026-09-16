#include <stdio.h>
#include <stdarg.h>
#include "fmi2Functions.h"

#if defined _WIN32 || defined __CYGWIN__
  #define EXPORT __declspec(dllexport)
#else
  #if __GNUC__ >= 4
    #define EXPORT __attribute__ ((visibility ("default")))
  #else
    #define EXPORT
  #endif
#endif

static fmi2CallbackLogger s_logger = NULL;

static void log_message(fmi2ComponentEnvironment componentEnvironment, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...) {
    
    if (!s_logger) return;

    char* buffer = NULL;
    
    va_list args;

    va_start(args, message);
    int len = vsnprintf(buffer, 0, message, args);
    va_end(args);

    if (len < 0) return;

    buffer = (char*)malloc(len + 1);
  
    if (!buffer) return;

    va_start(args, message);
    len = vsnprintf(buffer, len + 1, message, args);
    va_end(args);

    if (len < 0) {
        free(buffer);
        return;
    }
    
    s_logger(componentEnvironment, instanceName, status, category, buffer);

    free(buffer);
}

EXPORT void add_logger_proxy(fmi2CallbackFunctions *functions) {
    if (functions->logger != log_message) {
        s_logger = functions->logger;
        functions->logger = log_message;
    }
}
