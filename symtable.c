/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xbalek01 Miroslav Bálek
 */

#include "symtable.h"
#include "utils.h"
#include "token.h"
#include "instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compare_keys(char *key1, char *key2)
{
    return SYMTABLE_MIN(SYMTABLE_MAX(strcmp(key1, key2), -1), 1);
}

variable_ptr variable_create(char *name, type_t type, bool type_nullable)
{
    variable_ptr variable = malloc(sizeof(struct variable_t));

    MALLOC_CHECK(variable);

    variable->name = malloc(sizeof(char) * (strlen(name) + 1));

    MALLOC_CHECK(variable->name);

    strcpy(variable->name, name);

    return variable;
}

char *parameter_to_string(parameter_t parameter)
{
    size_t len = snprintf(NULL, 0, "%s (%s%s)", parameter.name, parameter.type_nullable == true ? "?" : "", type_to_name(parameter.type));
    char *s = malloc(sizeof(char) * len + 1);

    MALLOC_CHECK(s);

    sprintf(s, "%s (%s%s)", parameter.name, parameter.type_nullable == true ? "?" : "", type_to_name(parameter.type));
    return s;
}

char *function_to_string(function_ptr function)
{
    // parameters to string
    char *str = malloc(1);

    MALLOC_CHECK(str);

    str[0] = '\0';

    for (int i = 0; i < function->parameter_count; i++)
    {
        char *s = parameter_to_string(function->parameters[i]);
        str = realloc(str, sizeof(char) * (strlen(str) + strlen(s) + 1));
        strcat(str, s);

        if (i < function->parameter_count - 1)
        {
            str = realloc(str, sizeof(char) * (strlen(str) + 3));
            strcat(str, ", ");
        }

        free(s);
    }

    size_t len = snprintf(NULL, 0, "%s (%s%s, [%s])",
                          function->name,
                          function->return_type_nullable ? "?" : "",
                          type_to_name(function->return_type),
                          str);

    char *s = malloc(sizeof(char) * len + 1);

    MALLOC_CHECK(s);

    sprintf(s, "%s (%s%s, [%s])",
            function->name,
            function->return_type_nullable ? "?" : "",
            type_to_name(function->return_type),
            str);

    free(str);

    return s;
}

char *variable_to_string(variable_ptr variable)
{
    size_t len = snprintf(NULL, 0, "%s", variable->name);
    char *s = malloc(sizeof(char) * (len + 1));

    MALLOC_CHECK(s);

    sprintf(s, "%s", variable->name);
    return s;
}

function_ptr function_create(char *name, type_t return_type, bool return_type_nullable)
{
    function_ptr function = malloc(sizeof(struct function_t));

    MALLOC_CHECK(function);

    function->name = malloc(sizeof(char) * (strlen(name) + 1));

    MALLOC_CHECK(function->name);

    strcpy(function->name, name);

    function->return_type = return_type;
    function->return_type_nullable = return_type_nullable;
    function->variadic = false;
    function->has_return = false;
    function->called = false;
    function->defined = false;

    function->parameter_count = 0;
    function->parameters = malloc(0);

    MALLOC_CHECK(function->parameters);

    function->symtable = sym_init();
    function->instr_buffer = instr_buffer_init(function->name);

    return function;
}

void sym_get_variables_rec(sym_node_ptr root, variable_ptr **vars, int *count)
{
    if (root == NULL)
    {
        return;
    }

    sym_get_variables_rec(root->left, vars, count);
    sym_get_variables_rec(root->right, vars, count);

    if (root->variable != NULL)
    {
        *count += 1;
        *vars = realloc(*vars, sizeof(variable_ptr) * *count);

        MALLOC_CHECK(*vars);
    }
}

// Get all the variables
variable_ptr *sym_get_variables(sym_table_ptr table, int *count)
{
    variable_ptr *vars = malloc(sizeof(variable_ptr) * 0);
    *count = 0;

    MALLOC_CHECK(vars);

    sym_get_variables_rec(table->root, &vars, count);

    return vars;
}

void sym_get_functions_rec(sym_node_ptr root, function_ptr **fns, int *count)
{
    if (root == NULL)
    {
        return;
    }

    sym_get_functions_rec(root->left, fns, count);
    sym_get_functions_rec(root->right, fns, count);

    if (root->function != NULL)
    {
        *count += 1;
        *fns = realloc(*fns, sizeof(function_ptr) * (*count));

        MALLOC_CHECK(*fns);

        (*fns)[*count - 1] = root->function;
    }
}

// Get all the functions
function_ptr *sym_get_functions(sym_table_ptr table, int *count)
{
    function_ptr *fns = malloc(sizeof(function_ptr) * 0);

    MALLOC_CHECK(fns);

    *count = 0;

    sym_get_functions_rec(table->root, &fns, count);

    return fns;
}

void append_parameter(function_ptr function, char *name, type_t type, bool type_nullable)
{
    char *p_name = malloc(sizeof(char) * (strlen(name) + 1));
    MALLOC_CHECK(p_name);
    p_name = strcpy(p_name, name);

    parameter_t parameter = {.type = type, .type_nullable = type_nullable, .name = p_name};

    function->parameter_count += 1;
    function->parameters = realloc(function->parameters, sizeof(parameter_t) * function->parameter_count);

    MALLOC_CHECK(function->parameters);

    function->parameters[function->parameter_count - 1] = parameter;
}

void function_dispose(function_ptr function)
{
    if (function == NULL)
        return;

    // Free local symtable
    sym_dispose(function->symtable);

    for (int i = 0; i < function->parameter_count; i++)
    {
        free(function->parameters[i].name);
    }

    free(function->name);
    free(function->parameters);
    free(function);
}

void variable_dispose(variable_ptr variable)
{
    if (variable == NULL)
        return;

    free(variable);
}

function_ptr sym_get_function(sym_table_ptr table, char *id)
{
    sym_node_ptr node = sym_search(table, id);
    return node == NULL ? NULL : node->function;
}

variable_ptr sym_get_variable(sym_table_ptr table, char *id)
{
    sym_node_ptr node = sym_search(table, id);
    return node == NULL ? NULL : node->variable;
}

sym_node_ptr create_node(char *id, function_ptr function, variable_ptr variable)
{
    sym_node_ptr node = malloc(sizeof(struct sym_node_t));

    MALLOC_CHECK(node);

    node->function = function;
    node->variable = variable;

    node->id = malloc(sizeof(char) * (strlen(id) + 1));

    MALLOC_CHECK(node->id);

    strcpy(node->id, id);

    node->left = NULL;
    node->right = NULL;
    return node;
}

sym_table_ptr sym_init()
{
    sym_table_ptr table = malloc(sizeof(struct sym_table_t));

    MALLOC_CHECK(table);

    table->root = NULL;

    return table;
}

sym_node_ptr sym_insert_rec(sym_node_ptr root, sym_node_ptr node)
{
    if (root == NULL)
    {
        return node;
    }
    else
    {
        if (compare_keys(node->id, root->id) == 1)
        {
            root->left = sym_insert_rec(root->left, node);
        }
        else if (compare_keys(node->id, root->id) == -1)
        {
            root->right = sym_insert_rec(root->right, node);
        }
        else
        {
            // only overwrite what's not null
            if (node->function != NULL)
                root->function = node->function;
            if (node->variable != NULL)
                root->variable = node->variable;
            free(node);
        }
    }
    return root;
}

void sym_insert(sym_table_ptr table, sym_node_ptr node)
{
    table->root = sym_insert_rec(table->root, node);
}

void sym_insert_fn(sym_table_ptr table, function_ptr function)
{
    sym_node_ptr node = create_node(function->name, function, NULL);
    sym_insert(table, node);
}

void sym_insert_var(sym_table_ptr table, variable_ptr variable)
{
    sym_node_ptr node = create_node(variable->name, NULL, variable);
    sym_insert(table, node);
}

sym_node_ptr sym_min(sym_node_ptr root)
{
    if (root == NULL)
    {
        return NULL;
    }
    return (root->left == NULL) ? root : sym_min(root->left);
}

void sym_delete_rec(sym_node_ptr root, char *id)
{
    if (root == NULL)
    {
        return;
    }

    if (compare_keys(id, root->id) == 1)
    {
        sym_delete_rec(root->left, id);
    }
    else if (compare_keys(id, root->id) == -1)
    {
        sym_delete_rec(root->right, id);
    }
    else
    {
        if (root->right == NULL && root->left == NULL) // no children
        {
            free(root->id);
            function_dispose(root->function);
            variable_dispose(root->variable);
            free(root);
        }
        else if (root->right != NULL && root->left != NULL) // both children
        {
            sym_node_ptr min = sym_min(root->right);

            root->function = min->function;
            root->variable = min->variable;

            root->id = min->id;
            sym_delete_rec(root->right, min->id);
        }
        else
        {
            free(root->id);
            function_dispose(root->function);
            variable_dispose(root->variable);
            free(root);
        }
    }
}

void node_dispose(sym_node_ptr node)
{
    function_dispose(node->function);
    variable_dispose(node->variable);
    free(node->id);
    free(node);
}

void sym_delete(sym_table_ptr table, char *id)
{
    sym_delete_rec(table->root, id);
}

void sym_dispose_rec(sym_node_ptr node)
{
    if (node == NULL)
    {
        return;
    }

    sym_dispose_rec(node->left);
    sym_dispose_rec(node->right);

    node_dispose(node);
}

void sym_dispose(sym_table_ptr table)
{
    sym_dispose_rec(table->root);
    free(table);
}

sym_node_ptr sym_search_rec(sym_node_ptr root, char *id)
{
    if (root == NULL)
        return NULL;

    if (compare_keys(id, root->id) == 1)
    {
        return sym_search_rec(root->left, id);
    }
    else if (compare_keys(id, root->id) == -1)
    {
        return sym_search_rec(root->right, id);
    }
    else
    {
        return root;
    }
}

sym_node_ptr sym_search(sym_table_ptr table, char *id)
{
    return sym_search_rec(table->root, id);
}

void sym_print_util(sym_node_ptr root, int space)
{
    if (root == NULL)
    {
        return;
    }

    space += SPACE_SIZE;

    sym_print_util(root->right, space);

    printf("\n");
    for (int i = SPACE_SIZE; i < space; i++)
        printf(" ");

    printf("%s<", root->id);

    sym_print_util(root->left, space);
}

void sym_print(sym_table_ptr table)
{
    printf("root\n");
    printf("↓");
    sym_print_util(table->root, 0);
    printf("\n ------------------------------------------------------------ \n");
}