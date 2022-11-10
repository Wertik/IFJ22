#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

string_ptr string_empty()
{
    string_ptr string = malloc(sizeof(struct string_t));

    if (string == NULL)
    {
        fprintf(stderr, "string_empty: malloc fail.\n");
        exit(1);
    }

    string->data = malloc(sizeof(char));

    if (string->data == NULL)
    {
        fprintf(stderr, "string_empty - data: malloc fail.\n");
        exit(1);
    }

    string->data[0] = '\0';
    string->size = 0;

    return string;
}

string_ptr string_create(char *data)
{
    string_ptr string = malloc(sizeof(struct string_t));

    if (string == NULL)
    {
        fprintf(stderr, "string_create: malloc fail.\n");
        exit(1);
    }

    string->data = data;
    string->size = strlen(data);

    return string;
}

void string_append(string_ptr string, char c)
{
    string->size = string->size + 1;
    string->data = realloc(string->data, string->size + 1); // null byte

    if (string->data == NULL)
    {
        fprintf(stderr, "string_append: malloc fail.\n");
        exit(1);
    }

    string->data[string->size - 1] = c;
    string->data[string->size] = '\0';
}

void string_print(string_ptr string)
{
    printf("Buffer (%d): %s\n", string->size, string->data);
}

void string_destroy(string_ptr string)
{
    string->data = malloc(sizeof(char));

    if (string->data == NULL)
    {
        fprintf(stderr, "string_empty - data: malloc fail.\n");
        exit(1);
    }

    string->data[0] = '\0';
    string->size = 0;
}

void string_dispose(string_ptr string)
{
    free(string->data);
    free(string);
}