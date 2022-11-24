#ifndef PARSER_H
#define PARSER_H

#include "array.h"
#include "symtable.h"
#include "stack.h"

// -- utilities

// Get token from stack.
token_ptr get_next_token(item_ptr *stack);

// Assert token type
void assert_token_type(token_ptr token, token_type_t token_type);

void assert_next_token(item_ptr *stack, token_type_t token_type);

void assert_n_tokens(item_ptr *stack, int count, ...);

// Assert token type and return the token
token_ptr assert_next_token_get(item_ptr *stack, token_type_t token_type);

// -- rule functions

void rule_prog(item_ptr *in_stack, table_node_ptr *tree);
void rule_prog_end(item_ptr *in_stack, table_node_ptr *tree);
void rule_statement(item_ptr *in_stack, table_node_ptr *tree, function_ptr function);
void rule_statement_list(item_ptr *in_stack, table_node_ptr *tree, function_ptr function);

void rule_argument_list_typ(item_ptr *in_stack, table_node_ptr *tree);
void rule_argument_next_typ(item_ptr *in_stack, table_node_ptr *tree);

void rule_argument_list(item_ptr *in_stack, table_node_ptr *tree);
void rule_argument_next(item_ptr *in_stack, table_node_ptr *tree);

// TODO: Redefine int to a expression result union type
type_t parse_expression(item_ptr *in_stack, table_node_ptr *tree);
void rule_expression_next(item_ptr *in_stack, table_node_ptr *tree);
table_node_ptr parse(array_ptr tokens);
#endif