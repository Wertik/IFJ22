/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 */

#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include "token.h"
#include "buffer.h"
#include <string.h>

// Generate an instruction with no operands
#define INSTRUCTION(buffer, instr)                                \
    do                                                            \
    {                                                             \
        instr_buffer_append(buffer, generate_instruction(instr)); \
    } while (0);

// Generate an instruction with count operands
#define INSTRUCTION_OPS(buffer, instr, count, ...)                        \
    do                                                                    \
    {                                                                     \
        char *call = generate_instruction_ops(instr, count, __VA_ARGS__); \
        instr_buffer_append(buffer, call);                                \
    } while (0);

#define INSTRUCTION_CMT(buffer, comment)                      \
    do                                                        \
    {                                                         \
        instr_buffer_append(buffer, alloc_str("# " comment)); \
    } while (0);

#define INSTRUCTION_CONV_ARG2_I2F(buffer)            \
    do                                               \
    {                                                \
        INSTRUCTION(instr_buffer, INSTR_INT2FLOATS); \
    } while (0);

#define INSTRUCTION_CONV_ARG1_I2F(buffer)                                         \
    do                                                                            \
    {                                                                             \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);                                  \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2")); \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));   \
        INSTRUCTION(buffer, INSTR_INT2FLOATS);                                    \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));  \
    } while (0);

#define FUNCTION_RETURN_TYPE_CHECK_LABEL(buffer, function_scope, called_function, label_cnt) \
    (dyn_str("%s%s_call_%s_%d_cont_typecheck", function_scope ? "_" : "", function_scope ? function_scope->name : "", called_function->name, label_cnt))

// Generate a unique label with the current buffer prefix and buffer cnt
#define INSTRUCTION_GEN_CTX_LABEL(buffer, buffer_cnt, label) \
    (dyn_str("%s%s_%d_%s", buffer->prefix == NULL ? "" : "_", buffer->prefix == NULL ? "" : buffer->prefix, buffer_cnt, label))

#define FUNCTION_RETURN_TYPE_CHECK(buffer, function_scope, called_function)                                                                \
    do                                                                                                                                     \
    {                                                                                                                                      \
        int label_cnt = buffer->len;                                                                                                       \
        INSTRUCTION_CMT(buffer, "Function return type check");                                                                             \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);                                                                                           \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_retval"));                                                        \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_retval_type"));                                                   \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_retval"));                                                          \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_retval_type"), instr_var(FRAME_TEMP, "_retval"));                   \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_retval_type"));                                                    \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_type_str(called_function->return_type));                                             \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQS, 1, FUNCTION_RETURN_TYPE_CHECK_LABEL(buffer, function_scope, called_function, label_cnt)); \
        INSTRUCTION_OPS(buffer, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_INVALID_RETURN_TYPE));                                        \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, FUNCTION_RETURN_TYPE_CHECK_LABEL(buffer, function_scope, called_function, label_cnt));     \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_retval"));                                                         \
        INSTRUCTION_CMT(buffer, "End function return type check");                                                                         \
    } while (0);

// Generate a function header
// LABEL <function>
// CREATEFRAME
// PUSHFRAME
#define FUNCTION_HEADER(buffer, name)                  \
    do                                                 \
    {                                                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, name); \
        INSTRUCTION(buffer, INSTR_CREATE_FRAME);       \
        INSTRUCTION(buffer, INSTR_PUSH_FRAME);         \
    } while (0);

// Generate a function return
// RETURN
#define FUNCTION_RETURN(buffer)            \
    do                                     \
    {                                      \
        INSTRUCTION(buffer, INSTR_RETURN); \
    } while (0);

// Generate a full function begin header.
// LABEL <function>
// CREATEFRAME
// PUSHFRAME
// (retval for non-void)
// DEFVAR TF@<param-name> (for each parameter)
// POPS TF@<param-name> (for each parameter)
#define FUNCTION_BEGIN(function)                                                                              \
    do                                                                                                        \
    {                                                                                                         \
        FUNCTION_HEADER(function->instr_buffer, alloc_str(function->name));                                   \
        for (int i = 0; i < function->parameter_count; i++)                                                   \
        {                                                                                                     \
            parameter_t parameter = function->parameters[i];                                                  \
            INSTRUCTION_OPS(function->instr_buffer, INSTR_DEFVAR, 1, instr_var(FRAME_LOCAL, parameter.name)); \
            INSTRUCTION_OPS(function->instr_buffer, INSTR_POPS, 1, instr_var(FRAME_LOCAL, parameter.name));   \
        }                                                                                                     \
    } while (0);

// Generate a full function end.
// POPS TF@_retval (for non-void)
// RETURN
#define FUNCTION_END(function)                                \
    do                                                        \
    {                                                         \
        INSTRUCTION(function->instr_buffer, INSTR_POP_FRAME); \
        FUNCTION_RETURN(function->instr_buffer);              \
    } while (0);

// Generate the built-in write function
#define BUILT_IN_WRITE(buffer)                                                                       \
    do                                                                                               \
    {                                                                                                \
        FUNCTION_HEADER(buffer, alloc_str("write"))                                                  \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$tmp"));                              \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$argcnt"));                           \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$argcnt"));                             \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_writeloop"));                            \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$argcnt"));                            \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("int@0"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQS, 1, alloc_str("_writeloop_end"));                    \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$tmp"));                                \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, instr_var(FRAME_LOCAL, "$tmp"));                     \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("nil@nil"));                               \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQS, 1, alloc_str("_writeloop_after_nil"));             \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 2, instr_var(FRAME_LOCAL, "$tmp"), instr_const_str("")); \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_writeloop_after_nil"));                  \
        INSTRUCTION_OPS(buffer, INSTR_WRITE, 1, alloc_str("LF@$tmp"));                               \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$argcnt"));                            \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("int@1"));                                 \
        INSTRUCTION(buffer, INSTR_SUBS);                                                             \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$argcnt"));                             \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_writeloop"));                             \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_writeloop_end"));                        \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                                        \
        FUNCTION_RETURN(buffer);                                                                     \
    } while (0);

#define BUILT_IN_READF(buffer)                                                 \
    do                                                                         \
    {                                                                          \
        FUNCTION_HEADER(buffer, alloc_str("readf"))                            \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));     \
        INSTRUCTION_OPS(buffer, INSTR_READ, 1, alloc_str("LF@$buffer float")); \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$buffer"));      \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                  \
        FUNCTION_RETURN(buffer);                                               \
    } while (0);

#define BUILT_IN_READI(buffer)                                               \
    do                                                                       \
    {                                                                        \
        FUNCTION_HEADER(buffer, alloc_str("readi"))                          \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));   \
        INSTRUCTION_OPS(buffer, INSTR_READ, 1, alloc_str("LF@$buffer int")); \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$buffer"));    \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                \
        FUNCTION_RETURN(buffer);                                             \
    } while (0);

#define BUILT_IN_READS(buffer)                                                  \
    do                                                                          \
    {                                                                           \
        FUNCTION_HEADER(buffer, alloc_str("reads"))                             \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));      \
        INSTRUCTION_OPS(buffer, INSTR_READ, 1, alloc_str("LF@$buffer string")); \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$buffer"));       \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                   \
        FUNCTION_RETURN(buffer);                                                \
    } while (0);

#define BUILT_IN_SUBSTRING(buffer)                                                                   \
    do                                                                                               \
    {                                                                                                \
        FUNCTION_HEADER(buffer, alloc_str("substring"))                                              \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));                           \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$start"));                            \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$end"));                              \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$substring"));                        \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$length"));                           \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$compare"));                          \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$char"));                             \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$buffer"));                             \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$start"));                              \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$end"));                                \
        INSTRUCTION_OPS(buffer, INSTR_STRLEN, 1, alloc_str("LF@$length LF@$buffer"));                \
        INSTRUCTION_OPS(buffer, INSTR_LT, 1, alloc_str("LF@$compare LF@$start int@0"));              \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_invalid LF@$compare bool@true"))      \
        INSTRUCTION_OPS(buffer, INSTR_LT, 1, alloc_str("LF@$compare LF@$end int@0"));                \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_invalid LF@$compare bool@true"))      \
        INSTRUCTION_OPS(buffer, INSTR_LT, 1, alloc_str("LF@$compare LF@$end LF@$start"));            \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_invalid LF@$compare bool@true"))      \
        INSTRUCTION_OPS(buffer, INSTR_LT, 1, alloc_str("LF@$compare LF@$start LF@$length"));         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 1, alloc_str("_invalid LF@$compare bool@true"))     \
        INSTRUCTION_OPS(buffer, INSTR_LT, 1, alloc_str("LF@$compare LF@$end LF@$length"));           \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFNEQ, 1, alloc_str("_invalid LF@$compare bool@true"));    \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 1, alloc_str("LF@$substring string@\000"));              \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_substrloop"));                           \
        INSTRUCTION_OPS(buffer, INSTR_GETCHAR, 1, alloc_str("LF@$char LF@$buffer LF@$start"));       \
        INSTRUCTION_OPS(buffer, INSTR_CONCAT, 1, alloc_str("LF@$substring LF@$substring LF@$char")); \
        INSTRUCTION_OPS(buffer, INSTR_ADD, 1, alloc_str("LF@$start LF@$start int@1"));               \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_end LF@$start LF@$end"));             \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_substrloop"));                            \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_invalid"));                              \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 1, alloc_str("LF@$substring nil@nil"));                  \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_end"));                                  \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$substring"));                         \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                                        \
        FUNCTION_RETURN(buffer);                                                                     \
    } while (0);

#define BUILT_IN_CHR(buffer)                                                          \
    do                                                                                \
    {                                                                                 \
        FUNCTION_HEADER(buffer, alloc_str("chr"))                                     \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));            \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$char"));              \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$buffer"));              \
        INSTRUCTION_OPS(buffer, INSTR_INT2CHAR, 1, alloc_str("LF@$char LF@$buffer")); \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$char"));               \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                         \
        FUNCTION_RETURN(buffer);                                                      \
    } while (0);

#define BUILT_IN_ORD(buffer)                                                                   \
    do                                                                                         \
    {                                                                                          \
        FUNCTION_HEADER(buffer, alloc_str("ord"))                                              \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));                     \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$char"));                       \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$integer"));                    \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$length"));                     \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$buffer"));                       \
        INSTRUCTION_OPS(buffer, INSTR_STRLEN, 1, alloc_str("LF@$length LF@$buffer"));          \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_invalid LF@$length int@0"));    \
        INSTRUCTION_OPS(buffer, INSTR_STRI2INT, 1, alloc_str("LF@$integer LF@$buffer int@0")); \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$integer"));                     \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                             \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_invalid"));                        \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("int@0"));                           \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_end"));                            \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                                  \
        FUNCTION_RETURN(buffer);                                                               \
    } while (0);

#define BUILT_IN_STRLEN(buffer)                                                      \
    do                                                                               \
    {                                                                                \
        FUNCTION_HEADER(buffer, alloc_str("strlen"))                                 \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$count"));            \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));           \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$buffer"));             \
        INSTRUCTION_OPS(buffer, INSTR_STRLEN, 1, alloc_str("LF@$count LF@$buffer")); \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$count"));             \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                        \
        FUNCTION_RETURN(buffer);                                                     \
    } while (0);

#define BUILT_IN_FLOATVAL(buffer)                                                                  \
    do                                                                                             \
    {                                                                                              \
        FUNCTION_HEADER(buffer, alloc_str("floatval"))                                             \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));                         \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$type"));                           \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$actual"));                         \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$return"));                         \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$buffer"));                           \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 1, alloc_str("LF@$type LF@$buffer"));                  \
        INSTRUCTION_OPS(buffer, INSTR_GETCHAR, 1, alloc_str("LF@$actual LF@$type int@0"));         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_intfloat LF@$actual string@i"));    \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_floatfloat LF@$actual string@f"));  \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_boolfloat LF@$actual string@b"));   \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_boolfalse LF@$actual string@n"));   \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_intfloat"));                           \
        INSTRUCTION_OPS(buffer, INSTR_INT2FLOAT, 1, alloc_str("LF@$return LF@$buffer"));           \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_floatfloat"));                         \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 1, alloc_str("LF@$return LF@$buffer"));                \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_boolfloat"));                          \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_boolfalse LF@$buffer bool@false")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE 1, alloc_str("LF@$return float@0x1p+0"));               \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_boolfalse"));                          \
        INSTRUCTION_OPS(buffer, INSTR_MOVE 1, alloc_str("LF@$return float@0x0p+0"));               \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_end"));                                \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$return"));                          \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                                      \
        FUNCTION_RETURN(buffer);                                                                   \
    } while (0);

#define BUILT_IN_INTVAL(buffer)                                                                    \
    do                                                                                             \
    {                                                                                              \
        FUNCTION_HEADER(buffer, alloc_str("floatval"))                                             \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));                         \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$type"));                           \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$actual"));                         \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$return"));                         \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$buffer"));                           \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 1, alloc_str("LF@$type LF@$buffer"));                  \
        INSTRUCTION_OPS(buffer, INSTR_GETCHAR, 1, alloc_str("LF@$actual LF@$type int@0"));         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_intint LF@$actual string@i"));      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_floatint LF@$actual string@f"));    \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_boolint LF@$actual string@b"));     \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_boolint LF@$actual string@n"));     \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_intint"));                             \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 1, alloc_str("LF@$return LF@$buffer"));                \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_floatint"));                           \
        INSTRUCTION_OPS(buffer, INSTR_FLOAT2INT, 1, alloc_str("LF@$return LF@$buffer"));           \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_boolint"));                            \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_boolfalse LF@$buffer bool@false")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE 1, alloc_str("LF@$return int@1"));                      \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_boolfalse"));                          \
        INSTRUCTION_OPS(buffer, INSTR_MOVE 1, alloc_str("LF@$return int@0"));                      \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_end"));                                \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$return"));                          \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                                      \
        FUNCTION_RETURN(buffer);                                                                   \
    } while (0);

#define BUILT_IN_STRval(buffer)                                                                    \
    do                                                                                             \
    {                                                                                              \
        FUNCTION_HEADER(buffer, alloc_str("strval"))                                               \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$type"));                           \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$buffer"));                         \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$actual"));                         \
        INSTRUCTION_OPS(buffer, INSTR_DEFVAR, 1, alloc_str("LF@$return"));                         \
        INSTRUCTION_OPS(buffer, INSTR_POPS, 1, alloc_str("LF@$buffer"));                           \
        INSTRUCTION_OPS(buffer, INSTR_TYPE, 1, alloc_str("LF@$type LF@$buffer"));                  \
        INSTRUCTION_OPS(buffer, INSTR_GETCHAR, 1, alloc_str("LF@$actual LF@$type int@0"));         \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_boolfalse LF@$actual string@n"));   \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_strstr LF@$actual string@s"));      \
        INSTRUCTION_OPS(buffer, INSTR_JUMPIFEQ, 1, alloc_str("_boolfalse LF@$buffer bool@false")); \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 1, alloc_str("LF@$return string@1"));                  \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_boolfalse"));                          \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 1, alloc_str("LF@$return string@\000"));               \
        INSTRUCTION_OPS(buffer, INSTR_JUMP, 1, alloc_str("_end"));                                 \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_strstr"));                             \
        INSTRUCTION_OPS(buffer, INSTR_MOVE, 1, alloc_str("LF@$return LF@$buffer"));                \
        INSTRUCTION_OPS(buffer, INSTR_LABEL, 1, alloc_str("_end"));                                \
        INSTRUCTION_OPS(buffer, INSTR_PUSHS, 1, alloc_str("LF@$return"));                          \
        INSTRUCTION(buffer, INSTR_POP_FRAME);                                                      \
        FUNCTION_RETURN(buffer);                                                                   \
    }
while (0)
    ;

typedef enum
{
    // var symb
    INSTR_MOVE,

    // Frames

    INSTR_CREATE_FRAME,
    INSTR_POP_FRAME,
    INSTR_PUSH_FRAME,
    // var
    INSTR_DEFVAR,

    // Function calls

    // call
    INSTR_CALL,
    INSTR_RETURN,

    // Stack

    // symb
    INSTR_PUSHS,
    // var
    INSTR_POPS,
    INSTR_CLEARS,

    // Arithmetic

    // var symb symb
    INSTR_ADD,
    // var symb symb
    INSTR_SUB,
    // var symb symb
    INSTR_MUL,
    // var symb symb
    INSTR_DIV,
    // var symb symb
    INSTR_IDIV,

    INSTR_ADDS,
    INSTR_SUBS,
    INSTR_MULS,
    INSTR_DIVS,
    INSTR_IDIVS,

    // Comps

    // var symb symb
    INSTR_LT,
    // var symb symb
    INSTR_GT,
    // var symb symb
    INSTR_EQ,

    INSTR_LTS,
    INSTR_GTS,
    INSTR_EQS,

    // Logic

    // var symb symb
    INSTR_AND,
    // var symb symb
    INSTR_OR,
    // var symb symb
    INSTR_NOT,

    INSTR_ANDS,
    INSTR_ORS,
    INSTR_NOTS,

    // Conversions

    // var symb
    INSTR_INT2FLOAT,
    // var symb
    INSTR_FLOAT2INT,
    // var symb
    INSTR_INT2CHAR,
    // var symb symb
    INSTR_STRI2INT,

    INSTR_INT2FLOATS,
    INSTR_FLOAT2INTS,
    INSTR_INT2CHARS,
    INSTR_STRI2INTS,

    // I/O

    // var type
    INSTR_READ,
    // symb
    INSTR_WRITE,

    // Str

    // var symb symb
    INSTR_CONCAT,
    // var symb
    INSTR_STRLEN,
    // var symb symb
    INSTR_SETCHAR,
    // var symb symb
    INSTR_GETCHAR,

    // Type

    // var symb
    INSTR_TYPE,

    // Flow control

    // label
    INSTR_LABEL,
    // label
    INSTR_JUMP,
    // label symb symb
    INSTR_JUMPIFEQ,
    // label symb symb
    INSTR_JUMPIFNEQ,

    INSTR_JUMPIFEQS,
    INSTR_JUMPIFNEQS,

    // symb
    INSTR_EXIT,

    // Debug

    INSTR_BREAK,
    // symb
    INSTR_DPRINT,
} instruction_t;

typedef enum
{
    FRAME_LOCAL,
    FRAME_GLOBAL,
    FRAME_TEMP,
} frame_t;

typedef struct instr_buffer_t
{
    char **instructions;
    size_t len;
    char *prefix;
} * instr_buffer_ptr;

// Helper functions

// Convert enum instruction to a valid ifjcode22 instruction.
const char *instruction_to_str(instruction_t instr);

// Convert frame enum into a formal name (GF, TF, TF) for ifjcode22
const char *frame_to_formal(frame_t frame);

// Create a valid variable string for ifjcode22
char *instr_var(frame_t frame, char *name);

// Create a valid int constant string for ifjcode22
char *instr_const_int(int val);

char *instr_const_float(double val);

// Replace a target character in str with given replacement.
// Used to replace control characters inside strings with ifjcode22 escape sequences.
char *str_rep(char *str, char target, char *replacement);

// Create a valid string literal string for ifjcode22
// Replaces control characters with correct escape senquences.
// ex.: "Hello world!\n" -> "string@Hello\032world!\010"
char *instr_const_str(char *str);

char *instr_type_str(type_t type);

char *instr_const_bool(bool val);

// Create an alloc'd string using printf.
char *dyn_str(const char *fmt, ...);

// Generate an instruction with no operands.
char *generate_instruction(instruction_t instr);

// Generate an instruction call with n operands.
// Result: <INSTR> <OP1> <OP2> ...<OPN>
// varargs have to be alloc'd strings, they are freed inside the function.
char *generate_instruction_ops(instruction_t instr, int n, ...);

// Instruction buffer

// Initialize an empty buffer
instr_buffer_ptr instr_buffer_init(char *prefix);

// Append an instruction to the buffer
// instr has to be an alloc'd string, it's free'd when isntruction buffer gets disposed
void instr_buffer_append(instr_buffer_ptr instr_buffer, char *instr);

// Print the instruction buffer in an array fashion.
// instr_buffer(size)[instr1, ..., instrN]
void instr_buffer_print(instr_buffer_ptr instr_buffer);

// Output the instruction buffer to stdout as valid ifjcode22
void instr_buffer_out(instr_buffer_ptr instr_buffer);

// Dispose of the instruction buffer, free memory and instructions
void instr_buffer_dispose(instr_buffer_ptr instr_buffer);

#endif
