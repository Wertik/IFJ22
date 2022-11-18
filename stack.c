#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"
#include "token.h"

item_ptr stack_init()
{
    return NULL;
}

item_ptr stack_top(item_ptr stack)
{
    return stack;
}

item_ptr stack_push(item_ptr stack, symbol_ptr symbol)
{
    item_ptr item = malloc(sizeof(struct item_t));

    if (item == NULL)
    {
        fprintf(stderr, "stack_push: malloc fail.\n");
        exit(99);
    }

    item->symbol = symbol;
    item->next = stack;

    return item;
}

int stack_size(item_ptr stack)
{
    int i = 0;
    for (; stack != NULL; i++)
    {
        stack = stack->next;
    }
    return i;
}

void stack_print(item_ptr stack)
{
    item_ptr item = stack;

    // TODO prettify

    int size = stack_size(stack);

    printf("stack(%d) [", size);

    for (int i = 0; i < size; i++)
    {
        symbol_ptr symbol = item->symbol;

        char *symbol_s = symbol_to_string(symbol);
        if (i == size - 1)
        {
            printf("%s", symbol_s);
        }
        else
        {
            printf("%s, ", symbol_s);
        }
        free(symbol_s);

        item = item->next;
    }
    printf("]\n");
}

item_ptr stack_pop(item_ptr stack)
{
    item_ptr top = stack->next;
    free(stack);
    return top;
}

void stack_dispose(item_ptr stack)
{
    while (stack != NULL)
    {
        if (stack->symbol->terminal)
        {
            free(stack->symbol->token);
        }
        free(stack->symbol);
        item_ptr next = stack->next;
        free(stack);
        stack = next;
    }
}
