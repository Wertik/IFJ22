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

// A preparser doesn't modify the token stack. Only traverses through it.

extern sym_table_ptr global_table;

token_ptr passert_next_token_get(item_ptr *item, token_type_t token_type);

void parse_parameter(item_ptr *item, function_ptr function);
void parse_parameter_next(item_ptr *item, function_ptr function);
void parse_parameter_list(item_ptr *item, function_ptr function);

void parse_function_definitions(stack_ptr stack);

#endif