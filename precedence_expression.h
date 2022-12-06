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

#define EXPRESSION_EQS(buffer)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        int label_cnt = buffer->len;                                                                                   \
                                                                                                                       \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);                                                                       \
                                                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1"));                                      \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1_type"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg1"));                                        \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));                                      \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2_type"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));                                        \
                                                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg1"));   \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg2"));   \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1_type"));                                  \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2_type"));                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_check_fail")); \
                                                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1"));                                       \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));                                       \
        INSTRUCTION(buffer, INSTR_EQS);                                                                                \
                                                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_check_succ"));       \
                                                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_check_fail"));      \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(false));                                              \
                                                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_check_succ"));      \
    } while (0);

void expression_prec(stack_ptr in_stack, stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr);

void perform_addition(stack_ptr in_stack, stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr);
void perform_reduction(stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr_buffer);

// Get the position of a token in the precedence table
int get_pos_in_t(token_ptr token);

// Get first non terminal symbol
token_ptr get_first_non_E(stack_ptr stack);

void conversion(instr_buffer_ptr instr_buffer, token_ptr arg1, token_type_t operator, token_ptr arg2);
#endif