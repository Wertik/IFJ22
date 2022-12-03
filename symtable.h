#ifndef SYMTABLE_H
#define SYMTABLE_H

#define SPACE_SIZE 4

#include "token.h"
#include <stdbool.h>

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

  bool has_return;

  int parameter_count;
  parameter_t *parameters;

  // Local symtable for the function
  struct sym_table_t *symtable;

  // Function has variadic arguments
  bool variadic;
} * function_ptr;

typedef struct sym_node_t
{
  // function / variable identifier
  // lexi alphanum compare
  char *id;

  // Can hold a function and a variable at the same time.
  // We do this because variable and function names can collide -- it's not a semantic error though.
  function_ptr function;
  variable_ptr variable;

  struct sym_node_t *left;
  struct sym_node_t *right;
} * sym_node_ptr;

typedef struct sym_table_t
{
  sym_node_ptr root;
} * sym_table_ptr;

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

function_ptr sym_get_function(sym_table_ptr table, char *id);
variable_ptr sym_get_variable(sym_table_ptr table, char *id);

sym_table_ptr sym_init();
sym_node_ptr sym_search(sym_table_ptr table, char *id);

sym_node_ptr create_node(char *id, function_ptr function, variable_ptr variable);

void sym_insert(sym_table_ptr table, char *id, function_ptr function, variable_ptr variable);

sym_node_ptr sym_min(sym_node_ptr root);

void sym_delete(sym_table_ptr table, char *id);

void sym_dispose(sym_table_ptr table);

void sym_print(sym_table_ptr table);
void sym_print_util(sym_node_ptr table, int spaces);

#endif