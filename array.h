#ifndef ARRAY_H
#define ARRAY_H

#include "token.h"

typedef struct element_t
{
    struct element_t *next;
    struct element_t *prev;
    token_ptr token;
} * element_ptr;

typedef struct array_t
{
    element_ptr first;
    element_ptr last;
    int size;
} * array_ptr;

/* Array functions */

array_ptr array_create();

void array_append(array_ptr array, token_ptr token);

// TODO: All the array functions? delete, etc.

void array_print(array_ptr array);

void array_dispose(array_ptr array);

/* Element functions */

element_ptr element_create(token_ptr token);

#endif