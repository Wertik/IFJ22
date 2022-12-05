/*
 * Project: IFJ22 language compiler
 *
 * @author xsynak03 Maroš Synák
 * @author xotrad00 Martin Otradovec
 */

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

//#include "array.h"
#include "symtable.h"
#include "stack.h"
#include "token.h"
#include "instruction.h"

// Get the position of a token in the precedence table
int get_pos_in_t(token_ptr token);

void perform_addition(stack_ptr in_stack, stack_ptr push_down_stack, sym_table_ptr table);
void perform_reduction(stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr);

// Get first non terminal symbol
token_ptr get_first_non_E(item_ptr stack);
int expression_prec(stack_ptr in_stack, stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr);
#endif