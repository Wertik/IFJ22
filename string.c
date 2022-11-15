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

// Ensure a new clean string.
string_ptr string_fresh(string_ptr str) {
    if (str == NULL) {
        return string_empty();
    }

    // Already clean
    if (str->size == 0) {
        return str;
    }

    // free the data buffer
    free(str->data);
    str->data = NULL;

    // Alloc a new data buffer
    string_clean(str);
    return str;
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

// Create a new data array for the string, don't free the current data buffer.
void string_clean(string_ptr string)
{
    string->data = malloc(sizeof(char));

    if (string->data == NULL)
    {
        fprintf(stderr, "string_destroy - data: malloc fail.\n");
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