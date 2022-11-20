#ifndef SYMTABLE_H
#define SYMTABLE_H

#define SPACE_SIZE 4

#include <stdbool.h>

typedef enum
{
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
} type_t;

typedef enum
{
  RETURN_TYPE_INT,
  RETURN_TYPE_FLOAT,
  RETURN_TYPE_STRING,
  RETURN_TYPE_VOID,
} return_type_t;

typedef struct variable_t
{
  type_t type;
  bool type_nullable;
} * variable_ptr;

typedef struct
{
  char *name;
  type_t type;
  bool type_nullable;
} parameter_t;

typedef struct function_t
{
  type_t return_type;
  bool return_type_nullable;

  int parameter_count;
  parameter_t *parameters;
} * function_ptr;

typedef struct table_node_t
{
  // function / variable identifier
  // lexi alphanum compare
  char *id;

  // Can hold a function and a variable at the same time.
  // We do this because variable and function names can collide -- it's not a semantic error though.
  function_ptr function;
  variable_ptr variable;

  struct table_node_t *left;
  struct table_node_t *right;
} * table_node_ptr;

// Lexi compare two keys
int compare_keys(char *key1, char *key2);

/* Variable management */
variable_ptr variable_create(type_t type, bool type_nullable);
char *variable_to_string(char *s, variable_ptr variable);
void variable_dispose(variable_ptr variable);

/* Function management */
function_ptr function_create();
char *function_to_string(char *id, function_ptr function);
void function_dispose();

// Append a parameter to the function
void append_parameter(function_ptr function, char *name, type_t type, bool type_nullable);

function_ptr sym_get_function(table_node_ptr root, char *id);
variable_ptr sym_get_variable(table_node_ptr root, char *id);

table_node_ptr sym_init();
table_node_ptr sym_search(table_node_ptr root, char *id);

table_node_ptr create_node(char *id, function_ptr function, variable_ptr variable);

table_node_ptr sym_insert(table_node_ptr root, char *id, function_ptr function, variable_ptr variable);

table_node_ptr sym_min(table_node_ptr root);
table_node_ptr sym_delete(table_node_ptr root, char *id);

void sym_dispose(table_node_ptr root);

void sym_print(table_node_ptr root);
void sym_print_util(table_node_ptr root, int spaces);

#endif