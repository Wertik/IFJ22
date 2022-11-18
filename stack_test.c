#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "grammar.h"

int main()
{
    item_ptr stack = stack_init();

    symbol_ptr symbol = create_nonterminal(NONTERMINAL_PROG);

    stack = stack_push(stack, symbol);

    symbol = create_nonterminal(NONTERMINAL_STATEMENT);
    stack = stack_push(stack, symbol);

    symbol = create_terminal(TOKEN_COMMA);
    stack = stack_push(stack, symbol);

    stack_print(stack);

    item_ptr item = stack_top(stack);
    symbol_print(item->symbol);

    stack = stack_pop(stack);

    stack_print(stack);

    stack = stack_pop(stack);

    stack_print(stack);

    stack_dispose(stack);
    return 0;
}
