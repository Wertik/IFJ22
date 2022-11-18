#include "array.h"
#include <stdlib.h>
#include <stdio.h>

array_ptr array_create()
{
    array_ptr array = malloc(sizeof(struct array_t));

    if (array == NULL)
    {
        // TODO: Handle better?
        return NULL;
    }

    array->first = NULL;
    array->last = NULL;
    array->size = 0;

    return array;
}

void array_append(array_ptr array, token_ptr token)
{
    element_ptr element = element_create(token);

    element_ptr last = array->last;

    if (last == NULL)
    {
        // array empty
        array->first = element;
    }
    else
    {
        last->next = element;
    }

    array->last = element;
    element->prev = last;

    array->size += 1;
}

element_ptr element_create(token_ptr token)
{
    element_ptr element = malloc(sizeof(struct element_t));

    if (element == NULL)
    {
        // TODO: Handle better?
        fprintf(stderr, "malloc fail.\n");
        exit(99);
    }

    element->next = NULL;
    element->prev = NULL;
    element->token = token;

    return element;
}

// Print an array of tokens
void array_print(array_ptr array)
{
    element_ptr element = array->first;

    printf("array(%d) [", array->size);
    // use a for purely to avoid the trailing comma
    for (int i = 0; i < array->size && element != NULL; i++)
    {
        token_ptr token = element->token;

        char *s = token_to_string(token);

        // last elem, no comma
        if (i == array->size - 1)
        {
            printf("%s", s);
        }
        else
        {
            printf("%s, ", s);
        }
        free(s);
        element = element->next;
    }
    printf("]\n");
}

void array_dispose(array_ptr array)
{
    element_ptr element = array->first;

    while (element != NULL)
    {
        element_ptr next = element->next;

        token_dispose(element->token);
        free(element);

        element = next;
    }

    free(array);
}
