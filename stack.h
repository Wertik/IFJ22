/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xbalek01 Miroslav Bálek
 */

#ifndef _STACK_H
#define _STACK_H

#include "symbol.h"

// Stack item
typedef struct item_t
{
  symbol_ptr symbol;
  struct item_t *next;
} * item_ptr;

typedef struct stack_t
{
  item_ptr last;
  item_ptr top;
} * stack_ptr;

// Initialize a stack
stack_ptr stack_init();

// Add an item to the end of the stack
void stack_add(stack_ptr stack, symbol_ptr symbol);
// Push an item to the top of the stack
void stack_push(stack_ptr stack, symbol_ptr symbol);
item_ptr stack_pop(stack_ptr stack);
item_ptr stack_top(stack_ptr stack);

bool stack_empty(stack_ptr stack);

unsigned int stack_size(stack_ptr stack);

void stack_print(stack_ptr stack);
void stack_dispose(stack_ptr stack);

#endif