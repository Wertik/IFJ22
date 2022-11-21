#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "parser.h"
#include "symtable.h"
#include "stack.h"
#include "utils.h"

token_ptr peek_top(item_ptr *stack)
{
    symbol_ptr symbol = stack_top(*stack)->symbol;
    return symbol->token;
}

token_ptr peek_exact_type(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = peek_top(stack);
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

int parse_expression(item_ptr *in_stack)
{
    DEBUG_RULE();

    token_ptr const_int = assert_next_token_get(in_stack, TOKEN_CONST_INT);

    int value = const_int->value.integer;
    token_dispose(const_int);
    return value;
}

// <statement> -> var_id = const_int;
// <statement> -> return <expression>;
// <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}
// <statement> -> while (<expression>) {<statement-list>}
// <statement> -> function id(<argument-list>) {<statement-list>}
void rule_statement(item_ptr *in_stack, table_node_ptr tree)
{
    DEBUG_RULE();

    token_ptr next = get_next_token(in_stack);

    if (next->type == TOKEN_VAR_ID)
    {
        // <statement> -> var_id = <expression>;

        assert_next_token(in_stack, TOKEN_ASSIGN);

        int value = parse_expression(in_stack);

        // TODO: Create symboltable entry

        DEBUG_OUTF("%s <- %d", next->value.string, value);

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

            int value = parse_expression(in_stack);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_OUTF("if (%d)", value);

            rule_statement_list(in_stack, tree);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_OUT("end if");

            // else { <statement-list> }

            assert_next_keyword(in_stack, KEYWORD_ELSE);

            assert_next_token(in_stack, TOKEN_LC_BRACKET);

            DEBUG_OUT("else");

            rule_statement_list(in_stack, tree);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_OUT("end else");
            break;
        case KEYWORD_WHILE:
            // <statement> -> while (<expression>) {<statement-list>}
            // TODO: Implement
            fprintf(stderr, "Not implemented.\n");
            exit(42);
            break;
        case KEYWORD_FUNCTION:
            // <statement> -> function id(<argument-list>) {<statement-list>}
            // TODO: Implement
            fprintf(stderr, "Not implemented.\n");
            exit(42);
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
void rule_statement_list(item_ptr *in_stack, table_node_ptr tree)
{
    DEBUG_RULE();

    // Decide based on first? There's always at least one statement

    token_ptr next = peek_top(in_stack);

    if (next->type == TOKEN_KEYWORD || next->type == TOKEN_VAR_ID)
    {
        // <statement-list> -> <statement><statement-list>
        rule_statement(in_stack, tree);

        rule_statement_list(in_stack, tree);
    }
    else
    {
        // <statement-list> -> eps
    }
}

// <prog> -> <?php <statement> ?>
void rule_prog(item_ptr *in_stack, table_node_ptr tree)
{
    DEBUG_RULE();

    // TODO: Declare

    assert_next_token(in_stack, TOKEN_OPENING_TAG);

    token_ptr php = assert_next_token_get(in_stack, TOKEN_ID);

    if (strcmp(php->value.string, "php") != 0) {
        fprintf(stderr, "Wrong prolog.\n");
        exit(1); // TODO: Correct code.
    }

    rule_statement_list(in_stack, tree);

    assert_next_token(in_stack, TOKEN_CLOSING_TAG);
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

    rule_prog(&in_stack, tree);

    if (stack_size(in_stack) != 0)
    {
        fprintf(stderr, "SA failed, symbols left on the input stack.\n");
        exit(1); // TODO: Correct code
    }

    return tree;
}