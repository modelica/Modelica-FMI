#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "ModelicaUtilities.h"


void ModelicaFormatMessage(const char* string, ...) {

    va_list args;
    va_start(args, string);

    vprintf(string, args);

    va_end(args);
}

void ModelicaVFormatMessage(const char* string, va_list args) {
    vprintf(string, args);
}

void ModelicaError(const char* string) {
    fprintf(stderr, string);
}

void ModelicaFormatWarning(const char* string, ...) {

    va_list args;
    va_start(args, string);

    vprintf(string, args);
}

void ModelicaFormatError(const char* string, ...) {

    va_list args;
    va_start(args, string);

    vfprintf(stderr, string, args);

    va_end(args);
}
