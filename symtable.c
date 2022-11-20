#include "symtable.h"
#include "utils.h"
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
        exit(99);
    }

    variable->type = TYPE_INT;
    variable->type_nullable = false;

    return variable;
}

char *type_to_name(type_t type)
{
    switch (type)
    {
    case TYPE_INT:
        return "TYPE_INT";
    case TYPE_STRING:
        return "TYPE_STRING";
    case TYPE_FLOAT:
        return "TYPE_FLOAT";
    default:
        return "#unknown-type";
    }
}

char *return_type_to_name(return_type_t type)
{
    switch (type)
    {
    case RETURN_TYPE_INT:
        return "RETURN_TYPE_INT";
    case RETURN_TYPE_STRING:
        return "RETURN_TYPE_STRING";
    case RETURN_TYPE_FLOAT:
        return "RETURN_TYPE_FLOAT";
    case RETURN_TYPE_VOID:
        return "RETURN_TYPE_VOID";
    default:
        return "#unknown-return-type";
    }
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
                          return_type_to_name(function->return_type),
                          str);

    char *s = malloc(sizeof(char) * len + 1);

    MALLOC_CHECK(s);

    sprintf(s, "%s (%s%s, [%s])",
            id,
            function->return_type_nullable ? "?" : "",
            return_type_to_name(function->return_type),
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
        exit(99);
    }

    function->return_type = RETURN_TYPE_VOID;
    function->return_type_nullable = false;

    function->parameter_count = 0;
    function->parameters = malloc(0);

    if (function->parameters == NULL)
    {
        fprintf(stderr, "function_create(parameters): malloc fail\n");
        exit(99);
    }

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

function_ptr sym_get_function(table_node_ptr root, char *id)
{
    table_node_ptr node = sym_search(root, id);
    return node == NULL ? NULL : node->function;
}

variable_ptr sym_get_variable(table_node_ptr root, char *id)
{
    table_node_ptr node = sym_search(root, id);
    return node == NULL ? NULL : node->variable;
}

table_node_ptr create_node(char *id, function_ptr function, variable_ptr variable)
{
    table_node_ptr node = malloc(sizeof(struct table_node_t));

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

table_node_ptr sym_init()
{
    return NULL;
}

table_node_ptr sym_insert(table_node_ptr root, char *id, function_ptr function, variable_ptr variable)
{
    if (root == NULL)
    {
        return create_node(id, function, variable);
    }
    else
    {
        if (compare_keys(id, root->id) == 1)
        {
            root->left = sym_insert(root->left, id, function, variable);
        }
        else if (compare_keys(id, root->id) == -1)
        {
            root->right = sym_insert(root->right, id, function, variable);
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

table_node_ptr sym_min(table_node_ptr root)
{
    if (root == NULL)
    {
        return NULL;
    }
    return (root->left == NULL) ? root : sym_min(root->left);
}

table_node_ptr sym_delete(table_node_ptr root, char *id)
{
    if (root == NULL)
    {
        return NULL;
    }
    else
    {
        if (compare_keys(id, root->id) == 1)
        {
            root->left = sym_delete(root->left, id);
            return root;
        }
        else if (compare_keys(id, root->id) == -1)
        {
            root->right = sym_delete(root->right, id);
            return root;
        }
        else
        {
            if (root->right == NULL && root->left == NULL) // no children
            {
                free(root->id);
                function_dispose(root->function);
                variable_dispose(root->variable);
                free(root);
                return NULL;
            }
            else if (root->right != NULL && root->left != NULL) // both children
            {
                table_node_ptr min = sym_min(root->right);

                root->function = min->function;
                root->variable = min->variable;

                root->id = min->id;
                root->right = sym_delete(root->right, min->id);
                return root;
            }
            else
            {
                table_node_ptr child = (root->left == NULL) ? root->right : root->left;
                free(root->id);
                function_dispose(root->function);
                variable_dispose(root->variable);
                free(root);
                return child;
            }
        }
    }
}

void sym_dispose(table_node_ptr root)
{
    char *id = root->id;
    while (root != NULL)
    {
        root = sym_delete(root, id);
        if (root != NULL)
        {
            id = root->id;
        }
    }
}

table_node_ptr sym_search(table_node_ptr root, char *id)
{
    if (root == NULL)
        return NULL;

    if (compare_keys(id, root->id) == 1)
    {
        return sym_search(root->left, id);
    }
    else if (compare_keys(id, root->id) == -1)
    {
        return sym_search(root->right, id);
    }
    else
    {
        return root;
    }
}

void sym_print_util(table_node_ptr root, int space)
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

void sym_print(table_node_ptr root)
{
    printf("root\n");
    printf("↓");
    sym_print_util(root, 0);
    printf("\n ------------------------------------------------------------ \n");
}