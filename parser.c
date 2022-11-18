#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "symtable.h"
#include "stack.h"

token_ptr expect_token_get(item_ptr *stack, token_type_t token_type) {
    symbol_ptr symbol = stack_top(*stack)->symbol;
    token_ptr token = symbol->token;

    if (token->type != token_type)
    {
        fprintf(stderr, "%s expected.\n", token_type_to_name(token_type));
        exit(1); // Return bool and bubble up instead?
    }

    *stack = stack_pop(*stack);
    free(symbol);
    return token;
}

void expect_token(item_ptr *stack, token_type_t token_type) {
    symbol_ptr symbol = stack_top(*stack)->symbol;
    token_ptr token = symbol->token;

    if (token->type != token_type)
    {
        fprintf(stderr, "%s expected.\n", token_type_to_name(token_type));
        exit(1); // Return bool and bubble up instead?
    }

    *stack = stack_pop(*stack);

    symbol_dispose(symbol);
    token_dispose(token);
}

// <statement> -> var_id = const_int
void rule_statement(item_ptr *in_stack, tree_node_ptr tree) {

    // TODO: Multiple rules in the future, decide based on Follow & First?

    // TODO: Create entry in symboltable
    token_ptr variable = expect_token_get(in_stack, TOKEN_VAR_ID);

    expect_token(in_stack, TOKEN_ASSIGN);

    // Assign value in symboltable
    token_ptr const_int = expect_token_get(in_stack, TOKEN_CONST_INT);

    expect_token(in_stack, TOKEN_SEMICOLON);

    printf("%s assigned a value of %d\n", variable->value.string, const_int->value.integer);

    token_dispose(variable);
    token_dispose(const_int);
}

// <statement-list> -> <statement>;<statement-list>
// <statement-list> -> eps
void rule_statement_list(item_ptr *in_stack, tree_node_ptr tree)
{
    rule_statement(in_stack, tree);
}

// <prog> -> <?php <statement> ?>
void rule_prog(item_ptr *in_stack, tree_node_ptr tree)
{
    expect_token(in_stack, TOKEN_LESS);
    expect_token(in_stack, TOKEN_NULLABLE);
    expect_token(in_stack, TOKEN_ID);

    rule_statement_list(in_stack, tree);

    expect_token(in_stack, TOKEN_NULLABLE);
    expect_token(in_stack, TOKEN_MORE);
}

tree_node_ptr parse(array_ptr tokens)
{
    tree_node_ptr tree = tree_init();

    item_ptr in_stack = stack_init();

    // fill input stack from tokens
    element_ptr element = tokens->last;
    while (element != NULL)
    {
        symbol_ptr symbol = create_terminal(element->token);
        in_stack = stack_push(in_stack, symbol);
        element = element->prev;
    }

    stack_print(in_stack);

    rule_prog(&in_stack, tree);

    if (stack_size(in_stack) != 0) {
        fprintf(stderr, "SA failed, symbols left on the input stack.\n");
        exit(1); // TODO: Correct code
    }

    return tree;
}