/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 */

#ifndef _PREPARSER_H
#define _PREPARSER_H

#include "stack.h"
#include "symtable.h"
#include "parser.h"

// peacefully assert next token type
#define PASSERT_NEXT_TOKEN(item, token_type)      \
    do                                            \
    {                                             \
        move(item);                               \
        token_ptr token = (*item)->symbol->token; \
        ASSERT_TOKEN_TYPE(token, token_type);     \
    } while (0);

// Use global_table from parser.
extern sym_table_ptr global_table;

// A preparser doesn't modify the token stack. Only traverses through it.
// Hence "peaceful" -- doesn't pop the stack.

// Traverse through the token stack peacefully and look for function headers.
// Save definitions to the global symbol table.
// Function definition: function id(<parameter-list>): type
void parse_function_definitions(stack_ptr stack);

// Parse a function header peacefully.
// function: function id(<parameter-list>): type
void parse_function(item_ptr *item);

// Parse a parameter list peacefully.
void parse_parameter_list(item_ptr *item, function_ptr function);
// Parse the next parameter peacefully.
void parse_parameter_next(item_ptr *item, function_ptr function);
// Parse parameter peacefully
void parse_parameter(item_ptr *item, function_ptr function);

// Peacefully assert the next token.
token_ptr passert_next_token_get(item_ptr *item, token_type_t token_type);

// Move pointer to the next token.
bool move(item_ptr *item);

#endif