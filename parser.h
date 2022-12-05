/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xsynak03 Maroš Synák
 * @author xdobes22 Kristián Dobeš
 */

#ifndef _PARSER_H
#define _PARSER_H

#include "symtable.h"
#include "stack.h"
#include "utils.h"
#include "instruction.h"

#define ASSERT_TOKEN_TYPE(token, token_type)                                                                                                     \
    do                                                                                                                                           \
    {                                                                                                                                            \
        DEBUG_ASSERT(token_type, token);                                                                                                         \
        if (token == NULL || token->type != token_type)                                                                                          \
        {                                                                                                                                        \
            fprintf(stderr, "%s expected, got %s.\n", token_type_to_name(token_type), token == NULL ? "NULL" : token_type_to_name(token->type)); \
            exit(FAIL_SYNTAX);                                                                                                                   \
        }                                                                                                                                        \
    } while (0);

#define ASSERT_NEXT_TOKEN(stack, token_type)     \
    do                                           \
    {                                            \
        token_ptr token = get_next_token(stack); \
        ASSERT_TOKEN_TYPE(token, token_type);    \
        if (token != NULL)                       \
            token_dispose(token);                \
    } while (0);

#define ASSERT_KEYWORD(stack, keyword_type)                                                                          \
    do                                                                                                               \
    {                                                                                                                \
        token_ptr token = assert_next_token_get(stack, TOKEN_KEYWORD);                                               \
        if (token->value.keyword != keyword_type)                                                                    \
        {                                                                                                            \
            fprintf(stderr, "Expected keyword %s, got %s.\n", #keyword_type, keyword_to_name(token->value.keyword)); \
            exit(FAIL_SYNTAX);                                                                                       \
        }                                                                                                            \
        token_dispose(token);                                                                                        \
    } while (0);

#define ASSERT_ID(stack, id)                                      \
    do                                                            \
    {                                                             \
        token_ptr token = assert_next_token_get(stack, TOKEN_ID); \
        if (strcmp(token->value.string, id) != 0)                 \
        {                                                         \
            fprintf(stderr, "Expected %s.\n", id);                \
            exit(FAIL_SYNTAX);                                    \
        }                                                         \
        token_dispose(token);                                     \
    } while (0);

#define STACK_THROW(stack)                    \
    do                                        \
    {                                         \
        token_dispose(get_next_token(stack)); \
    } while (0);

extern sym_table_ptr global_table;

// -- utilities

// Peek next nth token in the stack.
token_ptr peek(stack_ptr stack, int n);

// Peek next token
token_ptr peek_top(stack_ptr stack);

// Peek and look for an exact type. If the token has a different type return null.
token_ptr peek_exact_type(stack_ptr stack, token_type_t token_type);

// Get next token from the stack, pop it.
token_ptr get_next_token(stack_ptr stack);

// Assert token type and return the token
token_ptr assert_next_token_get(stack_ptr stack, token_type_t token_type);

void assert_token_type(token_ptr token, token_type_t token_type);

void assert_next_token(stack_ptr stack, token_type_t token_type);

void assert_n_tokens(stack_ptr stack, int count, ...);

// -- rule functions

void rule_prog(stack_ptr in_stack, sym_table_ptr sym_global, instr_buffer_ptr instr_buffer);

void rule_statement(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function, instr_buffer_ptr instr_buffer);
void rule_statement_list(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function, instr_buffer_ptr instr_buffer);

void rule_parameter(stack_ptr in_stack, function_ptr function);
void rule_parameter_list(stack_ptr in_stack, function_ptr function);
void rule_parameter_next(stack_ptr in_stack, function_ptr function);

void rule_argument(stack_ptr in_stack, sym_table_ptr table, parameter_t *parameter, instr_buffer_ptr instr_buffer);
int rule_argument_list(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function, instr_buffer_ptr instr_buffer, bool variadic);
int rule_argument_next(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function, instr_buffer_ptr instr_buffer, int current_parameter, bool variadic);

void rule_expression(stack_ptr in_stack, sym_table_ptr table, instr_buffer_ptr instr_buffer);

void parse(stack_ptr tokens);
#endif