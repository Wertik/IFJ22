#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "array.h"
#include "symtable.h"
#include "stack.h"

int get_pos_in_t(token_ptr TOKEN);
void perform_addition(item_ptr *push_down_stack, table_node_ptr *tree , item_ptr *in_stack);
void perform_reduction(item_ptr *push_down_stack, table_node_ptr *tree);

token_ptr get_first_non_E(item_ptr *stack);

int expression(item_ptr *push_down_stack, table_node_ptr *tree);
#endif