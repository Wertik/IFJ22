#ifndef ROOT_STACK_H
#define ROOT_STACK_H

#include "symtable.h"
#include <stdbool.h>

typedef struct elem_t
{
  struct table_node_t *root;
  struct elem_t *next;
}* elem_ptr;

typedef struct root_stack_t
{
  struct elem_t *top;  
}* root_stack_ptr;



root_stack_ptr root_stack_init();
void root_stack_push(root_stack_ptr stack, table_node_ptr root);
void root_stack_pop(root_stack_ptr stack);
elem_ptr root_stack_top(root_stack_ptr stack);
bool root_stack_is_empty(root_stack_ptr stack);
void root_stack_dispose(root_stack_ptr stack);

#endif