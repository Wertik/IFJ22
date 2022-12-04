/*
 * Project: IFJ22 language compiler
 *
 * @author xsynak03 Maroš Synák
 * @author xotrad00 Martin Otradovec
 */

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "array.h"
#include "symtable.h"
#include "stack.h"
#include "token.h"

int get_pos_in_t(token_ptr TOKEN);
void perform_addition(stack_ptr push_down_stack, sym_table_ptr tree , stack_ptr in_stack);
void perform_reduction(stack_ptr push_down_stack, sym_table_ptr tree);

token_ptr get_first_non_E(item_ptr stack);
int expression(stack_ptr push_down_stack, sym_table_ptr tree);
#endif