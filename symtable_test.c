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
            exit(1);                                       \
        }                                                  \
    } while (0);

int main()
{
    table_node_ptr root = sym_init();

    // Insert a function

    function_ptr function = function_create();
    append_parameter(function, "key", TYPE_INT, false);

    function->return_type = RETURN_TYPE_VOID;

    root = sym_insert(root, "hello", function, NULL);

    sym_print(root);

    function = sym_get_function(root, "hello");

    ASSERT(function != NULL, "function not null");
    ASSERT(function->parameter_count == 1, "function parameter count");
    ASSERT(function->parameters[0].type == TYPE_INT, "parameter type");
    ASSERT(strcmp(function->parameters[0].name, "key") == 0, "parameter name");

    variable_ptr variable = variable_create(TYPE_INT, false);
    root = sym_insert(root, "a", NULL, variable);

    sym_print(root);

    variable = sym_get_variable(root, "a");

    ASSERT(variable != NULL, "variable not null");

    variable = variable_create(TYPE_INT, false);

    root = sym_insert(root, "hello", NULL, variable);

    variable = sym_get_variable(root, "hello");

    ASSERT(variable != NULL, "variable not null 2");

    function = sym_get_function(root, "hello");

    char *s = function_to_string("hello", function);
    printf("%s\n", s);
    free(s);

    ASSERT(function != NULL, "function not null 2");

    sym_dispose(root);
}
