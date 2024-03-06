#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "ModelicaUtilities.h"


void ModelicaFormatMessage(const char* string, ...) {
	va_list vl;
	va_start(vl, string);
	vprintf(string, vl);
	va_end(vl);
}

void ModelicaVFormatMessage(const char* string, va_list vl) {
	vprintf(string, vl);
}

void ModelicaFormatError(const char* string, ...) {
	va_list vl;
	va_start(vl, string);
	vprintf(string, vl);
	va_end(vl);
	exit(1);
}
