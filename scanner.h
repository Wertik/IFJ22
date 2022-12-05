/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xbalek01 Miroslav Bálek
 */

#ifndef _SCANNER_H
#define _SCANNER_H

#include "token.h"
#include "stack.h"
#include "buffer.h"

// Carriage return
#define CR 13

/* States */
#define SCANNER_START 1
#define SCANNER_STRING 2
#define SCANNER_VAR_ID_START 3
#define SCANNER_STRING_ESCAPE 4
#define SCANNER_HEX_START 5
#define SCANNER_HEX_FIRST 6
#define SCANNER_OCTA_1 7
#define SCANNER_OCTA_2 8
#define SCANNER_ASIGN 9
#define SCANNER_EQ_START 10
#define SCANNER_EXCL_MARK 11
#define SCANNER_NOT_EQ_START 12
#define SCANNER_NUM_INT 13
#define SCANNER_NUM_POINT_START 14
#define SCANNER_NUM_DOUBLE 15
#define SCANNER_NUM_EXP_START 16
#define SCANNER_NUM_EXP_SIGN 17
#define SCANNER_NUM_EXP 18
#define SCANNER_LESS_THAN 19
#define SCANNER_MORE_THAN 20
#define SCANNER_DIVIDE 21
#define SCANNER_LINE_COMM 22
#define SCANNER_BLOCK_COMM 23
#define SCANNER_BLOCK_END 24
#define SCANNER_ID 25
#define SCANNER_VAR_ID 26
#define SCANNER_PROG_START 27
#define SCANNER_NULLABLE 28
#define SCANNER_EPILOG 29

// Change the scanner state
#define CHANGE_STATE(state)                 \
    do                                      \
    {                                       \
        DEBUG_STATE(*scanner_state, state); \
        *scanner_state = state;             \
    } while (0);

// Append a token with empty value
#define APPEND_EMPTY(stack, token_type)                          \
    do                                                           \
    {                                                            \
        token_value_t value;                                     \
        token_ptr token = token_create(token_type, NONE, value); \
        symbol_ptr symbol = create_terminal(token);              \
        stack_add(stack, symbol);                                \
    } while (0);

// Append a token with string value to the stack
// Reset the buffer
#define APPEND_STRING(stack, token_type, buffer)                                            \
    do                                                                                      \
    {                                                                                       \
        symbol_ptr symbol = create_terminal(token_create_string(token_type, buffer->data)); \
        stack_add(stack, symbol);                                                           \
        buffer_reset(buffer);                                                               \
    } while (0);

// Append a token with integer value to the stack
#define APPEND_INT(stack, token_type, value)                                                 \
    do                                                                                       \
    {                                                                                        \
        token_value_t token_value = {.integer = value};                                      \
        symbol_ptr symbol = create_terminal(token_create(token_type, INTEGER, token_value)); \
        stack_add(stack, symbol);                                                            \
    } while (0);

// Append a token with float value to the stack
#define APPEND_FLOAT(stack, token_type, value)                                             \
    do                                                                                     \
    {                                                                                      \
        token_value_t token_value = {.float_value = value};                                \
        symbol_ptr symbol = create_terminal(token_create(token_type, FLOAT, token_value)); \
        stack_add(stack, symbol);                                                          \
    } while (0);

// Append a keyword token the the stack
#define APPEND_KEYWORD(stack, keyword_type)                                      \
    do                                                                           \
    {                                                                            \
        symbol_ptr symbol = create_terminal(token_create_keyword(keyword_type)); \
        stack_add(stack, symbol);                                                \
    } while (0);

// Append a type token to the stack
#define APPEND_TYPE(stack, type)                                      \
    do                                                                \
    {                                                                 \
        symbol_ptr symbol = create_terminal(token_create_type(type)); \
        stack_add(stack, symbol);                                     \
    } while (0);

// Read stdin and parse input into tokens, save them onto a stack and return it.
stack_ptr tokenize();

// Parse a single character c.
// Work with the state of the scanner and parse into tokens, append them to stack.
bool parse_character(stack_ptr stack, buffer_ptr buffer, int *scanner_state, char c, int line, int character);

// Attempt to parse keyword from buffer.
// Reset the buffer and return true if successful.
bool attempt_keyword(stack_ptr stack, buffer_ptr buffer, char *keyword_str, keyword_t keyword);

// Attempt to parse type from buffer.
// Reset the buffer and return true if successful.
bool attempt_type(stack_ptr stack, buffer_ptr buffer, char *type_str, type_t type);

// Attempt to parse "null" from buffer
// Reset the buffer and return true if successful.
bool attempt_null(stack_ptr stack, buffer_ptr buffer);

// Attempt to "greedily" (reads extra characters and rolls back if unsuccessful) parse a string from stdin.
// Returns true if successful.
bool attempt_greedy(char *rest, bool ignore_whitespace);

#endif