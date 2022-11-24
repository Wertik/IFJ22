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
    symbol_ptr symbol = stack_top(*stack)->symbol;
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
    DEBUG_ASSERT(type, token->type);

    if (token->type != type)
    {
        fprintf(stderr, "%s expected.\n", token_type_to_name(type));
        exit(1); // Return bool and bubble up instead?
    }
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
type_t parse_expression(item_ptr *in_stack, table_node_ptr *tree)
{
    DEBUG_RULE();

    type_t result_type;
    token_ptr next = peek_top(in_stack);

    switch (next->type)
    {
    case TOKEN_L_PAREN:
        assert_next_token(in_stack, TOKEN_L_PAREN);
        result_type = parse_expression(in_stack, tree);
        assert_next_token(in_stack, TOKEN_R_PAREN);
        break;

    case TOKEN_CONST_DOUBLE:
        assert_next_token(in_stack, TOKEN_CONST_DOUBLE);

        result_type = TYPE_FLOAT;
        rule_expression_next(in_stack, tree);
        break;
    case TOKEN_CONST_INT:

        assert_next_token(in_stack, TOKEN_CONST_INT);
        result_type = TYPE_INT;
        rule_expression_next(in_stack, tree);
        break;
    case TOKEN_STRING_LIT:

        assert_next_token(in_stack, TOKEN_STRING_LIT);
        // value = parse_expression(in_stack);

        // how to return string
        // value = next->value.string;

        result_type = TYPE_STRING;

        rule_expression_next(in_stack, tree);
        break;
    case TOKEN_VAR_ID:

        assert_next_token(in_stack, TOKEN_VAR_ID);

        // TODO: Check if var exists in symtable, get the type.

        // WILL HAVE TO WORK WITH SYMBOL TABLE
        // value = parse_expression(in_stack);
        // value = next->value.string;
        rule_expression_next(in_stack, tree);
        break;
    case TOKEN_ID:
        assert_next_token(in_stack, TOKEN_ID);

        // TODO: Check if function exists and get the return type from symtable.

        // value = parse_expression(in_stack);
        // value = next->value.string;
        assert_next_token(in_stack, TOKEN_L_PAREN);
        rule_argument_list(in_stack, tree);
        assert_next_token(in_stack, TOKEN_R_PAREN);
        break;
    default:
        fprintf(stderr, "Not a valid expression.\n");
        exit(1); // TODO: Correct code
    }

    token_dispose(next);
    return result_type;
}

void rule_expression_next(item_ptr *in_stack, table_node_ptr *tree)
{
    token_ptr next = peek_top(in_stack);
    // is it actually necessary?
    // int value;
    switch (next->type)
    {
    case TOKEN_PLUS:

        assert_next_token(in_stack, TOKEN_PLUS);
        parse_expression(in_stack, tree);
        break;
    case TOKEN_MINUS:

        assert_next_token(in_stack, TOKEN_MINUS);
        parse_expression(in_stack, tree);
        break;
    case TOKEN_MULTIPLE:

        assert_next_token(in_stack, TOKEN_MULTIPLE);
        parse_expression(in_stack, tree);
        break;
    case TOKEN_DIVIDE:

        assert_next_token(in_stack, TOKEN_DIVIDE);
        parse_expression(in_stack, tree);
        break;
    case TOKEN_DOT:

        assert_next_token(in_stack, TOKEN_DOT);
        parse_expression(in_stack, tree);
        break;
    default:
        fprintf(stderr, "non valid expression\n");
        exit(1);
    }
}
// <statement> -> var_id = <expression>;
// <statement> -> return <expression>;
// <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}
// <statement> -> while (<expression>) {<statement-list>}
// <statement> -> function id(<argument-list>) {<statement-list>}
// <statement> -> id(<argument-list>);
void rule_statement(item_ptr *in_stack, table_node_ptr *tree, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = get_next_token(in_stack);

    if (next->type == TOKEN_VAR_ID)
    {
        // <statement> -> var_id = <expression>;
        assert_next_token(in_stack, TOKEN_ASSIGN);

        int value = parse_expression(in_stack, tree);

        // Create symboltable entry if not already present
        if (sym_get_variable(*tree, next->value.string) == NULL)
        {
            // TODO: Infer type from value (requires PSA)
            // TODO-CHECK: Nullable by default?
            variable_ptr variable = variable_create(TYPE_INT, true);
            *tree = sym_insert(*tree, next->value.string, NULL, variable);
        }

        DEBUG_OUTF("%s <- %d", next->value.string, value);

        assert_next_token(in_stack, TOKEN_SEMICOLON);
    }
    else if (next->type == TOKEN_ID)
    {
        // <statement> -> id(<argument-list>);

        // TODO: Check that the function exists.
        // TODO: Generate call IFJcode22.

        assert_next_token(in_stack, TOKEN_L_PAREN);

        rule_argument_list(in_stack, tree);

        assert_next_token(in_stack, TOKEN_R_PAREN);
        assert_next_token(in_stack, TOKEN_SEMICOLON);
    }
    else if (next->type == TOKEN_KEYWORD)
    {
        // if / function / while

        switch (next->value.keyword)
        {
        case KEYWORD_IF:
            // <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}

            // if (<expression>) {<statement-list>}

            assert_next_token(in_stack, TOKEN_L_PAREN);

            int value = parse_expression(in_stack, tree);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_OUTF("if (%d)", value);

            rule_statement_list(in_stack, tree, function);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_OUT("end if");

            // else { <statement-list> }

            assert_next_keyword(in_stack, KEYWORD_ELSE);

            assert_next_token(in_stack, TOKEN_LC_BRACKET);

            DEBUG_OUT("else");

            rule_statement_list(in_stack, tree, function);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_OUT("end else");
            break;
        case KEYWORD_WHILE:
            // <statement> -> while (<expression>) {<statement-list>}
            assert_next_token(in_stack, TOKEN_L_PAREN);

            value = parse_expression(in_stack, tree);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_OUTF("while (%d)", value);

            rule_statement_list(in_stack, tree, function);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_OUT("end while");
            break;
        case KEYWORD_FUNCTION:
        {
            // <statement> -> function id(<argument-list>) {<statement-list>}

            // Save function to symtable
            token_ptr function_id = assert_next_token_get(in_stack, TOKEN_ID);

            function_ptr function = function_create();
            *tree = sym_insert(*tree, function_id->value.string, function, NULL);

            // Parse arguments

            assert_next_token(in_stack, TOKEN_L_PAREN);

            rule_argument_list_typ(in_stack, tree);

            assert_next_token(in_stack, TOKEN_R_PAREN);

            assert_next_token(in_stack, TOKEN_COLON);

            // Save return type to symtable

            token_ptr return_type = assert_next_token_get(in_stack, TOKEN_TYPE);

            function->return_type = return_type->value.type;

            assert_next_token(in_stack, TOKEN_LC_BRACKET);

            // Function statement list

            rule_statement_list(in_stack, tree, function);

            if (function->return_type != TYPE_VOID && function->has_return == false)
            {
                fprintf(stderr, "Missing return statement for non-void function %s.", function_id->value.string);
                exit(1); // TODO: Correct code
            }

            assert_next_token(in_stack, TOKEN_RC_BRACKET);
            token_dispose(function_id);
            break;
        }
        case KEYWORD_RETURN:
            value = parse_expression(in_stack, tree);

            // TODO: Check function type based on parse_expression result type
            if (function->return_type != TYPE_INT)
            {
                fprintf(stderr, "Invalid function return type.\n");
                exit(1); // TODO: Correct code
            }

            assert_next_token(in_stack, TOKEN_SEMICOLON);
            break;
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
void rule_statement_list(item_ptr *in_stack, table_node_ptr *tree, function_ptr function)
{
    DEBUG_RULE();

    // Decide based on first? There doesn't have to be a statement...

    token_ptr next = peek_top(in_stack);

    if ((next->type == TOKEN_KEYWORD) || next->type == TOKEN_VAR_ID || next->type == TOKEN_ID)
    {
        // <statement-list> -> <statement>;<statement-list>
        rule_statement(in_stack, tree, function);

        rule_statement_list(in_stack, tree, function);
    }
    else
    {
        // <statement-list> -> eps
        return;
    }
}

void rule_argument_list_typ(item_ptr *in_stack, table_node_ptr *tree)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);
    if (next->type == TOKEN_TYPE)
    {
        assert_next_token(in_stack, TOKEN_TYPE);
        assert_next_token(in_stack, TOKEN_VAR_ID);
        rule_argument_next_typ(in_stack, tree);
    }
}

void rule_argument_next_typ(item_ptr *in_stack, table_node_ptr *tree)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    if (next->type == TOKEN_COMMA)
    {
        assert_next_token(in_stack, TOKEN_COMMA);
        assert_next_token(in_stack, TOKEN_TYPE);
        assert_next_token(in_stack, TOKEN_VAR_ID);
        rule_argument_next_typ(in_stack, tree);
    }
}

void rule_argument_list(item_ptr *in_stack, table_node_ptr *tree)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);
    switch (next->type)
    {
    case TOKEN_VAR_ID:
        assert_next_token(in_stack, TOKEN_VAR_ID);
        rule_argument_next(in_stack, tree);

        break;
        // technicky oba case rovnake asi by bolo lepsie dat or ale neviem
    // predpokladam ze nedostanem do funkcie nejaky vzorec ale len cislo alebo string
    // nedoriesene v scannery pre float pravdepodobne

    // assuming constatnt expression is for all types
    case TOKEN_CONST_INT:
        assert_next_token(in_stack, TOKEN_CONST_INT);
        rule_argument_next(in_stack, tree);
        break;
    case TOKEN_CONST_DOUBLE:
        assert_next_token(in_stack, TOKEN_CONST_DOUBLE);
        rule_argument_next(in_stack, tree);
        break;
    // missing TOKEN_CONST_STRING MAYBE???
    default:
        break;
    }
}

void rule_argument_next(item_ptr *in_stack, table_node_ptr *tree)
{
    DEBUG_RULE();
    token_ptr next = peek_top(in_stack);
    DEBUG_OUTF("rule_argument_next %d\n", next->type);
    if (next->type == TOKEN_COMMA)
    {
        assert_next_token(in_stack, TOKEN_COMMA);
        rule_argument_list(in_stack, tree);
    }
}

// <prog> -> <?php <statement> ?>
void rule_prog(item_ptr *in_stack, table_node_ptr *tree)
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

    // Start parsing the main program body.

    rule_statement_list(in_stack, tree, NULL);

    token_ptr closing = peek_top(in_stack);

    // Closing tag optional

    if (closing != NULL)
    {
        assert_next_token(in_stack, TOKEN_CLOSING_TAG);
    }
}

void rule_prog_end(item_ptr *in_stack, table_node_ptr *tree)
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
    table_node_ptr tree = sym_init();

    item_ptr in_stack = stack_init();

    // fill input stack from tokens
    element_ptr element = tokens->last;
    while (element != NULL)
    {
        symbol_ptr symbol = create_terminal(element->token);
        in_stack = stack_push(in_stack, symbol);
        element = element->prev;
    }

    rule_prog(&in_stack, &tree);

    if (stack_size(in_stack) != 0)
    {
        fprintf(stderr, "SA failed, symbols left on the input stack.\n");
        exit(1); // TODO: Correct code
    }

    return tree;
}
