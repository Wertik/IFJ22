#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "array.h"
#include "symtable.h"
#include "stack.h"


void perform_addition(item_ptr *push_down_stack, table_node_ptr *tree , item_ptr *in_stack);
void perform_reduction(item_ptr *push_down_stack, table_node_ptr *tree);

int expression(item_ptr *push_down_stack, table_node_ptr *tree);
#endif