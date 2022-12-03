#include "symtable.h"
#include "utils.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) (a > b ? b : a)

int compare_keys(char *key1, char *key2)
{
    // compare keys by alphabet order? ascii value
    int len = MIN(strlen(key1), strlen(key2));

    for (int i = 0; i < len; i++)
    {
        if (key1[i] > key2[i])
        {
            return 1;
        }
        else if (key1[i] < key2[i])
        {
            return -1;
        }
    }

    // strings are the same
    return 0;
}

variable_ptr variable_create(type_t type, bool type_nullable)
{
    variable_ptr variable = malloc(sizeof(struct variable_t));

    if (variable == NULL)
    {
        fprintf(stderr, "variable_create: malloc fail\n");
        exit(FAIL_INTERNAL);
    }

    variable->type = TYPE_INT;
    variable->type_nullable = false;

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

char *function_to_string(char *id, function_ptr function)
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
                          id,
                          function->return_type_nullable ? "?" : "",
                          type_to_name(function->return_type),
                          str);

    char *s = malloc(sizeof(char) * len + 1);

    MALLOC_CHECK(s);

    sprintf(s, "%s (%s%s, [%s])",
            id,
            function->return_type_nullable ? "?" : "",
            type_to_name(function->return_type),
            str);

    free(str);

    return s;
}

char *variable_to_string(char *id, variable_ptr variable)
{
    size_t len = snprintf(NULL, 0, "%s%s %s", variable->type_nullable == true ? "?" : "", type_to_name(variable->type), id);
    char *s = malloc(sizeof(char) * len + 1);

    MALLOC_CHECK(s);

    sprintf(s, "%s%s %s", variable->type_nullable == true ? "?" : "", type_to_name(variable->type), id);
    return s;
}

function_ptr function_create()
{
    function_ptr function = malloc(sizeof(struct function_t));

    if (function == NULL)
    {
        fprintf(stderr, "function_create: malloc fail\n");
        exit(FAIL_INTERNAL);
    }

    function->return_type = TYPE_VOID;
    function->return_type_nullable = false;
    function->variadic = false;
    function->has_return = false;

    function->parameter_count = 0;
    function->parameters = malloc(0);

    if (function->parameters == NULL)
    {
        fprintf(stderr, "function_create(parameters): malloc fail\n");
        exit(FAIL_INTERNAL);
    }

    function->symtable = sym_init();

    return function;
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

sym_node_ptr sym_insert_rec(sym_node_ptr root, char *id, function_ptr function, variable_ptr variable)
{
    if (root == NULL)
    {
        return create_node(id, function, variable);
    }
    else
    {
        if (compare_keys(id, root->id) == 1)
        {
            root->left = sym_insert_rec(root->left, id, function, variable);
        }
        else if (compare_keys(id, root->id) == -1)
        {
            root->right = sym_insert_rec(root->right, id, function, variable);
        }
        else
        {
            // only overwrite what's not null
            if (function != NULL)
                root->function = function;
            if (variable != NULL)
                root->variable = variable;
        }
    }
    return root;
}

void sym_insert(sym_table_ptr table, char *id, function_ptr function, variable_ptr variable)
{
    table->root = sym_insert_rec(table->root, id, function, variable);
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