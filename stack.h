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

// Initialize an empty stack.
stack_ptr stack_init();

// Add an item to the bottom (end) of the stack
void stack_add(stack_ptr stack, symbol_ptr symbol);

// Push an item to the top (start) of the stack
void stack_push(stack_ptr stack, symbol_ptr symbol);

// Pop the item at the top of the stack.
// Doesn't free any memory.
item_ptr stack_pop(stack_ptr stack);

// Return the top of the stack.
item_ptr stack_top(stack_ptr stack);

// Return true if the stack is empty.
bool stack_empty(stack_ptr stack);

// Return the size of the stack.
unsigned int stack_size(stack_ptr stack);

// Print the stack with fashion.
void stack_print(stack_ptr stack);

// Dispose of the stack.
// Calls symbol_dispose on each symbol.
// Attempts to free all the memory allocated for the stack.
void stack_dispose(stack_ptr stack);

#endif