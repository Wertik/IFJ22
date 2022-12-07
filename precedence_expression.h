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

// Create a temp frame, load types of arguments
#define EXPRESSION_TEMP(buffer)                                                                                      \
    do                                                                                                               \
    {                                                                                                                \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);                                                                     \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1"));                                    \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1_type"));                               \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg1"));                                      \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));                                    \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2_type"));                               \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));                                      \
                                                                                                                     \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg1")); \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg2")); \
    } while (0);

// Generate a string argument check
// Throw an error (7) if any of the arguments are strings
#define EXPRESSION_STR_CHECK(buffer, label_cnt)                                                                                                                                   \
    do                                                                                                                                                                            \
    {                                                                                                                                                                             \
        INSTRUCTION_CMT(buffer, "string check");                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "after_str"), instr_var(FRAME_TEMP, "_arg1_type"), instr_type_str(TYPE_STRING)); \
        INSTRUCTION_OPS(buffer, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_EXPRE));                                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "after_str"), instr_var(FRAME_TEMP, "_arg2_type"), instr_type_str(TYPE_STRING)); \
        INSTRUCTION_OPS(buffer, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_EXPRE));                                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "after_str"));                                                                       \
        INSTRUCTION_CMT(buffer, "end string check");                                                                                                                              \
    } while (0);

#define EXPRESSION_ARGS_NIL2INT(buffer, label_cnt)                                                                                                                         \
    do                                                                                                                                                                     \
    {                                                                                                                                                                      \
        INSTRUCTION_CMT(buffer, "arg1: nil -> 0");                                                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "arg1_nil_after"), instr_var(FRAME_TEMP, "_arg1"), alloc_str("nil@nil")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg1"), instr_const_int(0));                                                                        \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg1_type"), instr_type_str(TYPE_INT));                                                             \
        INSTRUCTION_CMT(buffer, "end arg1: nil -> 0");                                                                                                                     \
                                                                                                                                                                           \
        INSTRUCTION_CMT(buffer, "arg2: nil -> 0");                                                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "arg1_nil_after"));                                                           \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "arg2_nil_after"), instr_var(FRAME_TEMP, "_arg2"), alloc_str("nil@nil")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg2"), instr_const_int(0));                                                                        \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg2_type"), instr_type_str(TYPE_INT));                                                             \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "arg2_nil_after"));                                                           \
        INSTRUCTION_CMT(buffer, "end arg2: nil -> 0");                                                                                                                     \
    } while (0);

// Do a float conversion on the arguments
// float + float -> float + float (nothing)
// int + int -> int + int (nothing)
// int + float -> float + float
// float + int -> float + float
#define EXPRESSION_ARGS_INT2FLOAT(buffer, label_cnt, operation_label)                                                                                                                        \
    do                                                                                                                                                                                       \
    {                                                                                                                                                                                        \
        INSTRUCTION_CMT(buffer, "Same type, no conversion");                                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, operation_label), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg2_type")); \
        INSTRUCTION_CMT(buffer, "End same type, no conversion");                                                                                                                             \
                                                                                                                                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "convert_arg2"), instr_var(FRAME_TEMP, "_arg2_type"), instr_type_str(TYPE_INT));             \
                                                                                                                                                                                             \
        INSTRUCTION_CMT(buffer, "arg1: int -> float");                                                                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg1"));                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, operation_label));                                                                               \
        INSTRUCTION_CMT(buffer, "end arg1: int -> float");                                                                                                                                   \
                                                                                                                                                                                             \
        INSTRUCTION_CMT(buffer, "arg2: int -> float");                                                                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "convert_arg2"));                                                                               \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg2"));                                                                         \
        INSTRUCTION_CMT(buffer, "end arg2: int -> float");                                                                                                                                   \
    } while (0);

// convert both arguments to float
#define EXPRESSION_ARGS_2FLOAT(buffer, label_cnt, operation_label)                                                                                                                  \
    do                                                                                                                                                                              \
    {                                                                                                                                                                               \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "convert_arg2"), instr_var(FRAME_TEMP, "_arg1_type"), instr_type_str(TYPE_FLOAT));  \
                                                                                                                                                                                    \
        INSTRUCTION_CMT(buffer, "arg1: int -> float");                                                                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg1"));                                                                \
        INSTRUCTION_CMT(buffer, "end arg1: int -> float");                                                                                                                          \
                                                                                                                                                                                    \
        INSTRUCTION_CMT(buffer, "arg2: int -> float");                                                                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "convert_arg2"));                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, operation_label), instr_var(FRAME_TEMP, "_arg2_type"), instr_type_str(TYPE_FLOAT)); \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg2"));                                                                \
        INSTRUCTION_CMT(buffer, "end arg2: int -> float");                                                                                                                          \
    } while (0);

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

// doenst do str > int, float
#define EXPRESSION_MORE(buffer)                                                                                                                                                                   \
    do                                                                                                                                                                                            \
    {                                                                                                                                                                                             \
        int label_cnt = buffer->len;                                                                                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);                                                                                                                                                  \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg_type"));                                                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1_type"));                                                                                                            \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                   \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2_type"));                                                                                                            \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                   \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg1"));                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg2"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_str("nil"));                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_not_null"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type")); \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_not_null"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_not_null"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type")); \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                        \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_not_null"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg2_type"));        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(INTEGER));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_float"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg1"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_float"));                                                                                 \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_float"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg2"));                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_float"));                                                                                 \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"));                                                                                     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                  \
        INSTRUCTION(instr_buffer, INSTR_GTS);                                                                                                                                                     \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(false));                                                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                             \
    } while (0);

#define EXPRESSION_MORE_EQUAL(buffer)                                                                                                                                                             \
    do                                                                                                                                                                                            \
    {                                                                                                                                                                                             \
        int label_cnt = buffer->len;                                                                                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);                                                                                                                                                  \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg_type"));                                                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg_res"));                                                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1_type"));                                                                                                            \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                   \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2_type"));                                                                                                            \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                   \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg1"));                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg2"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_str("nil"));                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_not_null"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type")); \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(INTEGER));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_float1"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));         \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_float(0));                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_float1"));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(NONE));                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_int1"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));           \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_int(0));                                                                                           \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_int1"));                                                                                        \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_str(""));                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_float1"));                                                                                     \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"));                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(true));                                                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_not_null"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_not_null"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type")); \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(INTEGER));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_float2"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));         \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_float(0));                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"), instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_float2"));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(NONE));                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_int2"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));           \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_int(0));                                                                                           \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"), instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_int2"));                                                                                        \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_str(""));                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"), instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_float2"));                                                                                     \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"));                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(true));                                                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_not_null"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg2_type"));        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(INTEGER));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_float"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg1"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_float"));                                                                                 \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_float"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg2"));                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_float"));                                                                                 \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"));                                                                                     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_GT, 3, instr_var(FRAME_TEMP, "_arg_res"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg1"));                                                  \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "try_equal"), instr_const_bool(false), instr_var(FRAME_TEMP, "_arg_res"));                        \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(true));                                                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "try_equal"));                                                                                       \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_EQ, 3, instr_var(FRAME_TEMP, "_arg_res"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg1"));                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"), instr_const_bool(false), instr_var(FRAME_TEMP, "_arg_res"));                        \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(true));                                                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(false));                                                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                             \
    } while (0);

#define EXPRESSION_LESS(buffer)                                                                                                                                                                   \
    do                                                                                                                                                                                            \
    {                                                                                                                                                                                             \
        int label_cnt = buffer->len;                                                                                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);                                                                                                                                                  \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg_type"));                                                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1_type"));                                                                                                            \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                   \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2_type"));                                                                                                            \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                   \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg1"));                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg2"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_str("nil"));                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_not_null"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type")); \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_not_null"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_not_null"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type")); \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                        \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_not_null"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg2_type"));        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(INTEGER));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_float"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg1"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_float"));                                                                                 \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_float"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg2"));                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_float"));                                                                                 \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"));                                                                                     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                  \
        INSTRUCTION(instr_buffer, INSTR_LTS);                                                                                                                                                     \
    } while (0);

#define EXPRESSION_LESS_EQUAL(buffer)                                                                                                                                                             \
    do                                                                                                                                                                                            \
    {                                                                                                                                                                                             \
        int label_cnt = buffer->len;                                                                                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);                                                                                                                                                  \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg_type"));                                                                                                             \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg_res"));                                                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1_type"));                                                                                                            \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg1"));                                                                                                                   \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2_type"));                                                                                                            \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));                                                                                                                   \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg1"));                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg2"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_str("nil"));                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_not_null"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type")); \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(INTEGER));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_float1"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));         \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_float(0));                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_float1"));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(NONE));                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_int1"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));           \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_int(0));                                                                                           \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_int1"));                                                                                        \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_str(""));                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_float1"));                                                                                     \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_is_ar2_is"));                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(true));                                                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_not_null"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_not_null"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type")); \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(INTEGER));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_float2"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));         \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_float(0));                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"), instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_float2"));                                                                                      \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(NONE));                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_int2"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));           \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_int(0));                                                                                           \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"), instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "not_int2"));                                                                                        \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_const_str(""));                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"), instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg_type"));      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_float2"));                                                                                     \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_is_ar1_is"));                                                                             \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(true));                                                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_not_null"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg2_type"));        \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(INTEGER));                                                                                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_float"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg1"));                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_float"));                                                                                 \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_float"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 2, instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg2"));                                                                              \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_float"));                                                                                 \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"));                                                                                     \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LT, 3, instr_var(FRAME_TEMP, "_arg_res"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg1"));                                                  \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "try_equal"), instr_const_bool(false), instr_var(FRAME_TEMP, "_arg_res"));                        \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(true));                                                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "try_equal"));                                                                                       \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_EQ, 3, instr_var(FRAME_TEMP, "_arg_res"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg1"));                                                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"), instr_const_bool(false), instr_var(FRAME_TEMP, "_arg_res"));                        \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(true));                                                                                                                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                              \
                                                                                                                                                                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "res_false"));                                                                                       \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_const_bool(false));                                                                                                                         \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "end"));                                                                                             \
    } while (0);

/* #define EXPRESSION_DOT(buffer)                                                                                                                                                                 \
    do                                                                                                                                                                                         \
    {
        int label_cnt = buffer->len;

        INSTRUCTION(buffer, INSTR_CREATE_FRAME);

        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg_type"));
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg_res"));
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1"));
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg1_type"));
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg1"));
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2_type"));
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));

        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg1"));
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg2"));
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg2_type"));

        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg_type"), instr_type_str(STRING));
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_string"), instr_var(FRAME_TEMP, "_arg1_type"), instr_var(FRAME_TEMP, "_arg_type"));

        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg1"), instr_const_str(""));

        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg1_string"));

        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_string"), instr_var(FRAME_TEMP, "_arg2_type"), instr_var(FRAME_TEMP, "_arg_type"));

        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg2"), instr_const_str(""));
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "type_arg2_string"));

        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(buffer, label_cnt, "types_equal"));

        INSTRUCTION_OPS(buffer, INSTR_CONCAT, 3, instr_var(FRAME_TEMP, "_arg_res"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg1"))
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg_res"));
    } while (0); */

void expression_prec(stack_ptr in_stack, stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr);

void perform_addition(stack_ptr in_stack, stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr);
void perform_reduction(stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr_buffer);

// Get the position of a token in the precedence table
int get_pos_in_t(token_ptr token);

// Get first non terminal symbol
token_ptr get_first_non_E(stack_ptr stack);

void conversion(instr_buffer_ptr instr_buffer, token_ptr arg1, token_type_t operator, token_ptr arg2);
#endif