#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"
#include "utils.h"
#include "token.h"
#include "grammar.h"

stack_ptr stack_init()
{
    stack_ptr stack = malloc(sizeof(struct stack_t));

    MALLOC_CHECK(stack);

    return stack;
}

item_ptr stack_top(stack_ptr stack)
{
    return stack->top;
}

void stack_add(stack_ptr stack, symbol_ptr symbol)
{
    item_ptr item = malloc(sizeof(struct item_t));

    MALLOC_CHECK(item);

    item->symbol = symbol;
    if (stack->top == NULL)
    {
        stack->top = item;
    }

    if (stack->last != NULL)
    {
        stack->last->next = item;
    }

    stack->last = item;
}

void stack_push(stack_ptr stack, symbol_ptr symbol)
{
    item_ptr item = malloc(sizeof(struct item_t));

    MALLOC_CHECK(item);

    item->symbol = symbol;

    item->next = stack->top;

    if (stack->last == NULL)
    {
        stack->last = item;
    }
    stack->top = item;
}

unsigned int stack_size(stack_ptr stack)
{
    item_ptr item = stack->top;
    int i = 0;
    for (; item != NULL; i++)
    {
        item = item->next;
    }
    return i;
}

void stack_print(stack_ptr stack)
{
    item_ptr item = stack->top;

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

item_ptr stack_pop(stack_ptr stack)
{
    item_ptr item = stack->top;

    stack->top = item->next;

    if (item == stack->last)
    {
        stack->last = NULL;
    }

    return item;
}

void stack_dispose(stack_ptr stack)
{
    item_ptr item = stack->top;
    while (item != NULL)
    {
        symbol_dispose(item->symbol);

        item_ptr next = item->next;
        free(item);
        item = next;
    }
    free(stack);
}
