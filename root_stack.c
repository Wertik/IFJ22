#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "root_stack.h"

root_stack_ptr root_stack_init()
{
    root_stack_ptr stack = malloc(sizeof(root_stack_ptr));

    if (stack == NULL)
    {
        fprintf(stderr, "root_stack_init: malloc fail.\n");
        exit(99);
    }

    stack->top = NULL;

    return stack;
}

void root_stack_push(root_stack_ptr stack, table_node_ptr root)
{
    if (stack == NULL)
    {
        printf("Given root_stack_ptr is NULL");
        return;
    }
    else
    {
        elem_ptr elem = malloc(sizeof(struct elem_t));
        if (elem == NULL)
        {
            fprintf(stderr, "root_stack_push: malloc fail.\n");
            exit(99);
        }

        elem->root = root;
        elem->next = stack->top;
        stack->top = elem;
    }
}

void root_stack_pop(root_stack_ptr stack)
{
    elem_ptr elem;
    if (stack->top != NULL)
    {
        elem = stack->top;
        stack->top = stack->top->next;
        free(elem);
    }
}


elem_ptr root_stack_top(root_stack_ptr stack)
{
    return (stack != NULL) ?  stack->top : NULL;
}

bool root_stack_is_empty(root_stack_ptr stack)
{
    return (stack->top == NULL);
}

void root_stack_dispose(root_stack_ptr stack)
{

    while (!root_stack_is_empty(stack))
    {
        root_stack_pop(stack);
    }
    free(stack);
}
