#include <stdio.h>
#include "stack.h"

StackItem *StackInit(void)
{
    return NULL;
}

int StackTop(StackItem *stack) { return stack->data; }

StackItem *StackPush(StackItem *stack, int data)
{
    StackItem *item = (StackItem *)malloc(sizeof(StackItem));

    item->data = data;
    item->next = stack;

    if (item == NULL)
    {
            fprintf(stderr, "stack_push - data: malloc fail.\n");
        exit(99);
    }
    return item;
};

StackItem *StackPop(StackItem *stack)
{
    StackItem *top = stack->next;
    free(stack);
    return top;
}

void StackDispose(StackItem *stack)
{
    while (stack != NULL)
    {
        StackPop(stack);
    }
}
