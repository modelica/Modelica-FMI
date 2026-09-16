#include <stdio.h>
#include <string.h>
#include "structured_variable_name.tab.h"

// Regenerate the lexer and parser:
// flex -o structured_variable_name.yy.c structured_variable_name.l
// bison -d -o structured_variable_name.tab.c structured_variable_name.y

void set_input_string(const char* in);

void end_lexical_scan(void);

void yyerror(char** error_message, const char* s) {
    *error_message = strdup(s);
}

char* validate_variable_name(const char* name) {
    set_input_string(name);
    char* error_message = NULL;
    yyparse(&error_message);
    end_lexical_scan();
    return error_message;
}
