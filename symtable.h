/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xbalek02 Miroslav Bálek
 */

#ifndef _SYMTABLE_H
#define _SYMTABLE_H

#include "token.h"
#include "instruction.h"
#include <stdbool.h>

#define SPACE_SIZE 4

// There's no simple min function in std?
#define SYMTABLE_MIN(a, b) (a > b ? b : a)

// There's no simple max function in std?
#define SYMTABLE_MAX(a, b) (a > b ? a : b)

typedef struct variable_t
{
  char *name;
} * variable_ptr;

typedef struct
{
  // Parameter name
  char *name;
  // Parameter type
  type_t type;
  // Parameter type is nullable
  bool type_nullable;
} parameter_t;

typedef struct function_t
{
  char *name;

  type_t return_type;
  bool return_type_nullable;

  bool has_return;

  int parameter_count;
  parameter_t *parameters;

  // Local symtable for the function
  struct sym_table_t *symtable;

  // Whether the function is called somewhere in code
  bool called;

  // Whether the function has been defined or just called.
  bool defined;

  // Function has variadic arguments
  bool variadic;

  bool builtin;

  // Instruction buffer for function code
  // Allows for functions to be generated after main body.
  instr_buffer_ptr instr_buffer;
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

// Alphabetically compare two string keys.
int compare_keys(char *key1, char *key2);

// Create a variable
variable_ptr variable_create(char *name);
// Convert variable to a string with fashion
char *variable_to_string(variable_ptr variable);
// Dispose of a variable
void variable_dispose(variable_ptr variable);

// Create a function
function_ptr function_create(char *name, type_t return_type, bool return_type_nullable);
// Append a parameter to the function
void append_parameter(function_ptr function, char *name, type_t type, bool type_nullable);
// Convert function to string with fashion
char *function_to_string(function_ptr function);
// Convert a parameter to string.
char *parameter_to_string(parameter_t parameter);
// Dispose of a function
void function_dispose(function_ptr function);

// Get a function from the symbol table
function_ptr sym_get_function(sym_table_ptr table, char *id);
// Get a variable from the symbol table
variable_ptr sym_get_variable(sym_table_ptr table, char *id);

// Get all variables from the symbol table
variable_ptr *sym_get_variables(sym_table_ptr table, int *count);
void sym_get_variables_rec(sym_node_ptr root, variable_ptr **vars, int *count);

// Get all variables from the symbol table
function_ptr *sym_get_functions(sym_table_ptr table, int *count);
void sym_get_functions_rec(sym_node_ptr root, function_ptr **fns, int *count);

// Initialize an empty symbol table
sym_table_ptr sym_init();

// Search for an item in the symbol table
sym_node_ptr sym_search(sym_table_ptr table, char *id);
sym_node_ptr sym_search_rec(sym_node_ptr root, char *id);

// Create a symbol table node with a function / variable.
sym_node_ptr create_node(char *id, function_ptr function, variable_ptr variable);

// Insert a function into the symbol table.
void sym_insert_fn(sym_table_ptr table, function_ptr function);
// Insert a variable into the symbol table.
void sym_insert_var(sym_table_ptr table, variable_ptr variable);

// Insert a node into the symbol tree.
void sym_insert(sym_table_ptr table, sym_node_ptr node);
sym_node_ptr sym_insert_rec(sym_node_ptr root, sym_node_ptr node);

sym_node_ptr sym_min(sym_node_ptr root);

// Delete a node from the symbol table
// Free all memory associateed with the node.
void sym_delete(sym_table_ptr table, char *id);
void sym_delete_rec(sym_node_ptr root, char *id);

// Dispose of a symbol tree node.
// Free all memory associated with the node.
void node_dispose(sym_node_ptr node);

// Dispose of the symbol table.
// Free all the memory associated with nodes.
void sym_dispose(sym_table_ptr table);
void sym_dispose_rec(sym_node_ptr node);

// Print the symbol table (BST) with fashion.
void sym_print(sym_table_ptr table);
// Print a subtree from root node with fashion. Helper fn for sym_print.
void sym_print_util(sym_node_ptr root, int spaces);

#endif