#ifndef STACK_H
#define STACK_H

#include "grammar.h"

typedef struct item_t
{
  symbol_ptr symbol;
  struct item_t *next;
} * item_ptr;

item_ptr stack_init();
item_ptr stack_push(item_ptr stack, symbol_ptr symbol);
item_ptr stack_pop(item_ptr stack);

item_ptr stack_top(item_ptr stack);

void stack_print(item_ptr stack);

void stack_dispose(item_ptr stack);

#endif