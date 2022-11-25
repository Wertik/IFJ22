#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "array.h"
#include "symtable.h"
#include "stack.h"

int expression(item_ptr *push_down_stack, table_node_ptr *tree);
#endif