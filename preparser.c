#include "preparser.h"
#include "symtable.h"
#include "utils.h"
#include "token.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

bool move(item_ptr *item)
{
    *item = (*item)->next;

    if (*item == NULL)
    {
        return false;
    }
    return true;
}

token_ptr passert_next_token_get(item_ptr *item, token_type_t token_type)
{
    move(item);

    // TODO: can segfault
    token_ptr token = (*item)->symbol->token;

    ASSERT_TOKEN_TYPE(token, token_type);

    return token;
}

// <par> -> type var_id
void parse_parameter(item_ptr *item, function_ptr function)
{
    token_ptr par_type = passert_next_token_get(item, TOKEN_TYPE);
    token_ptr par_id = passert_next_token_get(item, TOKEN_VAR_ID);

    // cannot use void as parameter type
    if (par_type->value.type == TYPE_VOID)
    {
        fprintf(stderr, "VOID is not a valid parameter type.\n");
        exit(FAIL_SYNTAX);
    }

    // Append parameter
    // TODO: Nullable?
    append_parameter(function, par_id->value.string, par_type->value.type, false);
}

// <par-list> -> eps
// <par-list> -> <par> <par-next>
void parse_parameter_list(item_ptr *item, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = (*item)->next->symbol->token;

    if (next->type == TOKEN_TYPE)
    {
        // <par-list> -> <par> <par-next>
        parse_parameter(item, function);
        parse_parameter_next(item, function);
    }
    else
    {
        // <arg-list> -> eps
        return;
    }
}

// <par-next> -> eps
// <par-next> -> , <par> <par-next>
void parse_parameter_next(item_ptr *item, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = (*item)->next->symbol->token;

    if (next->type == TOKEN_COMMA)
    {
        move(item);

        parse_parameter(item, function);
        parse_parameter_next(item, function);
    }
    else
    {
        // <par-next> -> eps
        return;
    }
}

// Parse the function header and save into global_table
void parse_function(item_ptr *item)
{
    // <statement> -> function id(<argument-list>) : type {<statement-list>}

    // Save function to symtable
    token_ptr function_id = passert_next_token_get(item, TOKEN_ID);

    // Check if the function is already defined

    if (sym_search(global_table, function_id->value.string) != NULL)
    {
        fprintf(stderr, "Function %s already defined.\n", function_id->value.string);
        exit(FAIL_SEMANTIC_FUNC_DEF);
    }

    function_ptr function = function_create(function_id->value.string, TYPE_VOID, false);
    sym_insert_fn(global_table, function);

    DEBUG_PSEUDO("function %s(...)", function_id->value.string);

    // Parse parameters

    PASSERT_NEXT_TOKEN(item, TOKEN_L_PAREN);

    parse_parameter_list(item, function);

    for (int i = 0; i < function->parameter_count; i++)
    {
        parameter_t parameter = function->parameters[i];

        DEBUG_PSEUDO("Parameter %d: %s %s", i, type_to_name(parameter.type), parameter.name);

        // Insert as variables into function local symtable
        variable_ptr parameter_var = variable_create(parameter.name, parameter.type, parameter.type_nullable);
        sym_insert_var(function->symtable, parameter_var);
    }

    PASSERT_NEXT_TOKEN(item, TOKEN_R_PAREN);

    PASSERT_NEXT_TOKEN(item, TOKEN_COLON);

    // Save return type to symtable

    token_ptr return_type = passert_next_token_get(item, TOKEN_TYPE);

    function->return_type = return_type->value.type;

    DEBUG_PSEUDO("Returns %s", type_to_name(function->return_type));
}

// Pre-parse function headers and append function definitions to global table.
void parse_function_definitions(stack_ptr stack)
{
    item_ptr item = stack->top;

    // Look for function keyword, parse function and save it?
    while (item != NULL)
    {
        token_ptr token = item->symbol->token;

        if (token->type == TOKEN_KEYWORD && token->value.keyword == KEYWORD_FUNCTION)
        {
            // found a function declaration
            parse_function(&item);
        };
        item = item->next;
    }
}