#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "symbol.h"
#include "token.h"

int main()
{
    stack_ptr stack = stack_init();

    symbol_ptr symbol = create_nonterminal(NONTERMINAL_PROG);

    stack_push(stack, symbol);

    symbol = create_nonterminal(NONTERMINAL_STATEMENT);
    stack_push(stack, symbol);

    token_value_t value = {.integer = 10};
    token_ptr token = token_create(TOKEN_CONST_INT, INTEGER, value);
    symbol = create_terminal(token);
    stack_push(stack, symbol);

    stack_print(stack);

    item_ptr item = stack_top(stack);
    symbol_print(item->symbol);

    stack_pop(stack);

    stack_print(stack);

    stack_pop(stack);

    stack_print(stack);

    stack_dispose(stack);
    return 0;
}
