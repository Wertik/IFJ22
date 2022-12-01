#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "utils.h"

#define ASSERT(what, name)                                 \
    do                                                     \
    {                                                      \
        if ((what) == false)                               \
        {                                                  \
            fprintf(stderr, "Assert " #name " failed.\n"); \
            exit(FAIL_LEXICAL);                                       \
        }                                                  \
    } while (0);

int main()
{
    sym_table_ptr table = sym_init();

    // Insert a function

    function_ptr function = function_create();
    append_parameter(function, "key", TYPE_INT, false);

    function->return_type = TYPE_VOID;

    sym_insert(table, "hello", function, NULL);

    sym_print(table);

    function = sym_get_function(table, "hello");

    ASSERT(function != NULL, "function not null");
    ASSERT(function->parameter_count == 1, "function parameter count");
    ASSERT(function->parameters[0].type == TYPE_INT, "parameter type");
    ASSERT(strcmp(function->parameters[0].name, "key") == 0, "parameter name");

    variable_ptr variable = variable_create(TYPE_INT, false);
    sym_insert(table, "a", NULL, variable);

    sym_print(table);

    variable = sym_get_variable(table, "a");

    ASSERT(variable != NULL, "variable not null");

    variable = variable_create(TYPE_INT, false);

    sym_insert(table, "hello", NULL, variable);

    variable = sym_get_variable(table, "hello");

    ASSERT(variable != NULL, "variable not null 2");

    function = sym_get_function(table, "hello");

    char *s = function_to_string("hello", function);
    printf("%s\n", s);
    free(s);

    ASSERT(function != NULL, "function not null 2");

    sym_dispose(table);
}
