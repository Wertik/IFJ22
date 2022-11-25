#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "symtable.h"
#include "stack.h"
#include "utils.h"

int different_parse = 0;

token_ptr peek_top(item_ptr *stack)
{
    item_ptr top = stack_top(*stack);

    if (top == NULL)
    {
        return NULL;
    }
    symbol_ptr symbol = stack_top(*stack)->symbol;
    return symbol->token;
}

token_ptr peek_exact_type(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = peek_top(stack);
    if (token == NULL)
    {
        return NULL;
    }
    return token->type == token_type ? token : NULL;
}

token_ptr get_next_token(item_ptr *stack)
{
    item_ptr top = stack_top(*stack);

    if (top == NULL || top->symbol == NULL) {
        return NULL;
    }

    symbol_ptr symbol = top->symbol;
    token_ptr token = symbol->token;

    *stack = stack_pop(*stack);

    DEBUG_TOKEN(token);
    DEBUG_STACK_TOP(*stack, 2);

    free(symbol);
    return token;
}

token_ptr assert_next_token_get(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = get_next_token(stack);

    assert_token_type(token, token_type);

    return token;
}

void assert_token_type(token_ptr token, token_type_t type)
{
    if (token == NULL || token->type != type)
    {
        fprintf(stderr, "%s expected.\n", token_type_to_name(type));
        exit(1); // Return bool and bubble up instead?
    }

    DEBUG_ASSERT(type, token->type);
}

void assert_next_token(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = get_next_token(stack);

    assert_token_type(token, token_type);

    token_dispose(token);
}

void assert_next_keyword(item_ptr *stack, keyword_t keyword)
{
    token_ptr token = assert_next_token_get(stack, TOKEN_KEYWORD);

    if (token->value.keyword != keyword)
    {
        fprintf(stderr, "Expected keyword %s, got %s.\n", keyword_to_name(keyword), keyword_to_name(token->value.keyword));
        exit(1); // TODO: Correct code
    }

    token_dispose(token);
}

void assert_next_id(item_ptr *stack, char *id)
{
    token_ptr token = assert_next_token_get(stack, TOKEN_ID);

    if (strcmp(token->value.string, id) != 0)
    {
        fprintf(stderr, "Expected %s.", id);
        exit(1); // TODO: Correct code
    }

    token_dispose(token);
}

void assert_n_tokens(item_ptr *stack, int n, ...)
{
    va_list valist;
    va_start(valist, n);

    for (int i = 0; i < n; i++)
    {
        assert_next_token(stack, va_arg(valist, token_type_t));
    }

    va_end(valist);
}

// treba asi zmenit
type_t parse_expression(item_ptr *in_stack, table_node_ptr *sym_global)
{
    DEBUG_RULE();

    type_t result_type;
    token_ptr next = peek_top(in_stack);

    switch (next->type)
    {
    case TOKEN_L_PAREN:
        assert_next_token(in_stack, TOKEN_L_PAREN);
        result_type = parse_expression(in_stack, sym_global);
        assert_next_token(in_stack, TOKEN_R_PAREN);
        break;

    case TOKEN_CONST_DOUBLE:
        assert_next_token(in_stack, TOKEN_CONST_DOUBLE);

        result_type = TYPE_FLOAT;
        rule_expression_next(in_stack, sym_global);
        break;
    case TOKEN_CONST_INT:
        assert_next_token(in_stack, TOKEN_CONST_INT);
        result_type = TYPE_INT;
        rule_expression_next(in_stack, sym_global);
        break;
    case TOKEN_STRING_LIT:

        assert_next_token(in_stack, TOKEN_STRING_LIT);
        // value = parse_expression(in_stack);

        // how to return string
        // value = next->value.string;

        result_type = TYPE_STRING;

        rule_expression_next(in_stack, sym_global);
        break;
    case TOKEN_VAR_ID:

        assert_next_token(in_stack, TOKEN_VAR_ID);

        // TODO: Check if var exists in symtable, get the type.

        // WILL HAVE TO WORK WITH SYMBOL TABLE
        // value = parse_expression(in_stack);
        // value = next->value.string;
        rule_expression_next(in_stack, sym_global);
        break;
    case TOKEN_ID:
        assert_next_token(in_stack, TOKEN_ID);

        // TODO: Check if function exists and get the return type from symtable.

        // value = parse_expression(in_stack);
        // value = next->value.string;
        assert_next_token(in_stack, TOKEN_L_PAREN);
        rule_argument_list(in_stack, sym_global);
        assert_next_token(in_stack, TOKEN_R_PAREN);
        break;
    default:
        fprintf(stderr, "Not a valid expression.\n");
        exit(1); // TODO: Correct code
    }

    return result_type;
}

void rule_expression_next(item_ptr *in_stack, table_node_ptr *sym_global)
{
    token_ptr next = peek_top(in_stack);
    // is it actually necessary?
    // int value;
    switch (next->type)
    {
    case TOKEN_PLUS:

        assert_next_token(in_stack, TOKEN_PLUS);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_MINUS:

        assert_next_token(in_stack, TOKEN_MINUS);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_MULTIPLE:

        assert_next_token(in_stack, TOKEN_MULTIPLE);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_DIVIDE:

        assert_next_token(in_stack, TOKEN_DIVIDE);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_DOT:

        assert_next_token(in_stack, TOKEN_DOT);
        parse_expression(in_stack, sym_global);
        break;
    default:
    {
    }
        // $x = <10>; -> 10 is a valid expression
        /* default:
            fprintf(stderr, "non valid expression\n");
            exit(1); */
    }
}

// <statement> -> var_id = <expression>;
// <statement> -> return <expression>;
// <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}
// <statement> -> while (<expression>) {<statement-list>}
// <statement> -> function id(<argument-list>) {<statement-list>}
// <statement> -> id(<argument-list>);
void rule_statement(item_ptr *in_stack, table_node_ptr *sym_global, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = get_next_token(in_stack);

    if (next->type == TOKEN_VAR_ID)
    {
        // <statement> -> var_id = <expression>;
        assert_next_token(in_stack, TOKEN_ASSIGN);

        int value = parse_expression(in_stack, sym_global);

        // Create symboltable entry if not already present
        if (sym_get_variable(*sym_global, next->value.string) == NULL)
        {
            // TODO: Infer type from value (requires PSA)
            // TODO-CHECK: Nullable by default?
            variable_ptr variable = variable_create(TYPE_INT, true);
            *sym_global = sym_insert(*sym_global, next->value.string, NULL, variable);
        }

        DEBUG_PSEUDOF("%s <- %d", next->value.string, value);

        assert_next_token(in_stack, TOKEN_SEMICOLON);
    }
    else if (next->type == TOKEN_ID)
    {
        // <statement> -> id(<argument-list>);

        // TODO: Check that the function exists.
        // TODO: Generate call IFJcode22.
        // TODO: Check parameter count and types

        assert_next_token(in_stack, TOKEN_L_PAREN);

        rule_argument_list(in_stack, sym_global);

        assert_next_token(in_stack, TOKEN_R_PAREN);
        assert_next_token(in_stack, TOKEN_SEMICOLON);
    }
    else if (next->type == TOKEN_KEYWORD)
    {
        // if / function / while

        switch (next->value.keyword)
        {
        case KEYWORD_IF:
        {
            // <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}

            // if (<expression>) {<statement-list>}

            assert_next_token(in_stack, TOKEN_L_PAREN);

            type_t type = parse_expression(in_stack, sym_global);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_PSEUDOF("if (%s)", type_to_name(type));

            rule_statement_list(in_stack, sym_global, function);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end if");

            // else { <statement-list> }

            assert_next_keyword(in_stack, KEYWORD_ELSE);

            assert_next_token(in_stack, TOKEN_LC_BRACKET);

            DEBUG_PSEUDO("else");

            rule_statement_list(in_stack, sym_global, function);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end else");
            break;
        }
        case KEYWORD_WHILE:
        {
            // <statement> -> while (<expression>) {<statement-list>}
            assert_next_token(in_stack, TOKEN_L_PAREN);

            type_t type = parse_expression(in_stack, sym_global);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_PSEUDOF("while (%s)", type_to_name(type));

            rule_statement_list(in_stack, sym_global, function);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end while");
            break;
        }
        case KEYWORD_FUNCTION:
        {
            // <statement> -> function id(<argument-list>) {<statement-list>}

            // Save function to symtable
            token_ptr function_id = assert_next_token_get(in_stack, TOKEN_ID);

            function_ptr function = function_create();
            *sym_global = sym_insert(*sym_global, function_id->value.string, function, NULL);

            DEBUG_PSEUDOF("function %s(...)", function_id->value.string);

            // Parse arguments

            assert_next_token(in_stack, TOKEN_L_PAREN);

            rule_argument_list_typ(in_stack, sym_global, function);

            for (int i = 0; i < function->parameter_count; i++) {
                DEBUG_PSEUDOF("Parameter %d: %s %s", i, type_to_name(function->parameters[i].type), function->parameters[i].name);
            }

            assert_next_token(in_stack, TOKEN_R_PAREN);

            assert_next_token(in_stack, TOKEN_COLON);

            // Save return type to symtable

            token_ptr return_type = assert_next_token_get(in_stack, TOKEN_TYPE);

            function->return_type = return_type->value.type;

            DEBUG_PSEUDOF("Returns %s", type_to_name(function->return_type));

            assert_next_token(in_stack, TOKEN_LC_BRACKET);

            // Function statement list

            rule_statement_list(in_stack, sym_global, function);

            if (function->return_type != TYPE_VOID && function->has_return == false)
            {
                fprintf(stderr, "Missing return statement for non-void function %s.", function_id->value.string);
                exit(1); // TODO: Correct code
            }

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDOF("end function %s", function_id->value.string);

            token_dispose(function_id);
            break;
        }
        case KEYWORD_RETURN:
        {
            type_t result_type = parse_expression(in_stack, sym_global);

            // TODO: Check function type based on parse_expression result type
            if (function->return_type != result_type)
            {
                fprintf(stderr, "Invalid function return type.\n");
                exit(1); // TODO: Correct code
            }

            function->has_return = true;

            DEBUG_PSEUDOF("return %s;", type_to_name(result_type));

            assert_next_token(in_stack, TOKEN_SEMICOLON);
            break;
        }
        default:
            fprintf(stderr, "Invalid keyword in statement.\n");
            exit(1); // TODO: Correct code
        }
    }
    else
    {
        fprintf(stderr, "Invalid token.\n");
        exit(1); // TODO: Correct code
    }

    token_dispose(next);
}

// <statement-list> -> <statement><statement-list>
// <statement-list> -> eps
void rule_statement_list(item_ptr *in_stack, table_node_ptr *sym_global, function_ptr function)
{
    DEBUG_RULE();

    // Decide based on first? There doesn't have to be a statement...

    token_ptr next = peek_top(in_stack);

    if (next == NULL) {
        return;
    }

    if ((next->type == TOKEN_KEYWORD) || next->type == TOKEN_VAR_ID || next->type == TOKEN_ID)
    {
        // <statement-list> -> <statement>;<statement-list>
        rule_statement(in_stack, sym_global, function);

        rule_statement_list(in_stack, sym_global, function);
    }
    else
    {
        // <statement-list> -> eps
        return;
    }
}

void rule_argument_list_typ(item_ptr *in_stack, table_node_ptr *sym_global, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    if (next ==  NULL) {
        return;
    }

    if (next->type == TOKEN_TYPE)
    {
        // TODO: Extract into function

        token_ptr par_type = assert_next_token_get(in_stack, TOKEN_TYPE);
        token_ptr par_id = assert_next_token_get(in_stack, TOKEN_VAR_ID);

        // cannot use void as parameter type
        if (par_type->value.type == TYPE_VOID)
        {
            fprintf(stderr, "VOID is not a valid type of arguments.\n");
            exit(1); // TODO: Correct code
        }

        // Append parameter
        // TODO: Nullable?
        append_parameter(function, par_id->value.string, par_type->value.type, false);

        token_dispose(par_type);
        token_dispose(par_id);

        rule_argument_next_typ(in_stack, sym_global, function);
    }
}

void rule_argument_next_typ(item_ptr *in_stack, table_node_ptr *sym_global, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    if (next->type == TOKEN_COMMA)
    {
        assert_next_token(in_stack, TOKEN_COMMA);

        // TODO: Extract into function

        token_ptr par_type = assert_next_token_get(in_stack, TOKEN_TYPE);
        token_ptr par_id = assert_next_token_get(in_stack, TOKEN_VAR_ID);

        // cannot use void as parameter type
        if (par_type->value.type == TYPE_VOID)
        {
            fprintf(stderr, "VOID is not a valid type of arguments.\n");
            exit(1); // TODO: Correct code
        }

        // Append parameter
        // TODO: Nullable?
        append_parameter(function, par_id->value.string, par_type->value.type, false);

        token_dispose(par_type);
        token_dispose(par_id);

        rule_argument_next_typ(in_stack, sym_global, function);
    }
}

void rule_argument_list(item_ptr *in_stack, table_node_ptr *sym_global)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);
    switch (next->type)
    {
    case TOKEN_VAR_ID:
        assert_next_token(in_stack, TOKEN_VAR_ID);
        rule_argument_next(in_stack, sym_global);

        break;
        // technicky oba case rovnake asi by bolo lepsie dat or ale neviem
    // predpokladam ze nedostanem do funkcie nejaky vzorec ale len cislo alebo string
    // nedoriesene v scannery pre float pravdepodobne

    // assuming constatnt expression is for all types
    case TOKEN_CONST_INT:
        assert_next_token(in_stack, TOKEN_CONST_INT);
        rule_argument_next(in_stack, sym_global);
        break;
    case TOKEN_CONST_DOUBLE:
        assert_next_token(in_stack, TOKEN_CONST_DOUBLE);
        rule_argument_next(in_stack, sym_global);
        break;
    // missing TOKEN_CONST_STRING MAYBE???
    default:
        break;
    }
}

void rule_argument_next(item_ptr *in_stack, table_node_ptr *sym_global)
{
    DEBUG_RULE();
    token_ptr next = peek_top(in_stack);
    DEBUG_PSEUDOF("rule_argument_next %d\n", next->type);
    if (next->type == TOKEN_COMMA)
    {
        assert_next_token(in_stack, TOKEN_COMMA);
        rule_argument_list(in_stack, sym_global);
    }
}

// <prog> -> <?php <statement> ?>
void rule_prog(item_ptr *in_stack, table_node_ptr *sym_global)
{
    DEBUG_RULE();

    // Parse prolog

    assert_next_token(in_stack, TOKEN_OPENING_TAG);

    assert_next_id(in_stack, "php");
    assert_next_id(in_stack, "declare");

    assert_next_token(in_stack, TOKEN_L_PAREN);

    assert_next_id(in_stack, "strict_types");
    assert_next_token(in_stack, TOKEN_ASSIGN);

    token_ptr const_int = assert_next_token_get(in_stack, TOKEN_CONST_INT);

    if (const_int->value.integer != 1)
    {
        fprintf(stderr, "Strict types has to be enabled.\n");
        exit(1); // TODO: Correct code
    }

    token_dispose(const_int);

    assert_next_token(in_stack, TOKEN_R_PAREN);

    assert_next_token(in_stack, TOKEN_SEMICOLON);

    // Start parsing the main program body.

    rule_statement_list(in_stack, sym_global, NULL);

    token_ptr closing = peek_top(in_stack);

    // Closing tag optional

    if (closing != NULL)
    {
        assert_next_token(in_stack, TOKEN_CLOSING_TAG);
    }
}

void rule_prog_end(item_ptr *in_stack, table_node_ptr *sym_global)
{
    token_ptr next = peek_top(in_stack);
    if (next->type == TOKEN_NULLABLE)
    {
        assert_next_token(in_stack, TOKEN_NULLABLE);
        assert_next_token(in_stack, TOKEN_MORE);
    }
}

table_node_ptr parse(array_ptr tokens)
{
    table_node_ptr sym_global = sym_init();

    item_ptr in_stack = stack_init();

    // fill input stack from tokens
    element_ptr element = tokens->last;
    while (element != NULL)
    {
        symbol_ptr symbol = create_terminal(element->token);
        in_stack = stack_push(in_stack, symbol);
        element = element->prev;
    }

    rule_prog(&in_stack, &sym_global);

    if (stack_size(in_stack) != 0)
    {
        fprintf(stderr, "SA failed, symbols left on the input stack.\n");
        exit(1); // TODO: Correct code
    }

    return sym_global;
}
