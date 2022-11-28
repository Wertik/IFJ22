#include "string.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

string_ptr string_empty()
{
    string_ptr string = malloc(sizeof(struct string_t));

    MALLOC_CHECK(string);

    string->data = malloc(sizeof(char));

    MALLOC_CHECK(string->data);

    string->data[0] = '\0';
    string->size = 0;

    return string;
}

// Ensure a new clean string.
string_ptr string_fresh(string_ptr str)
{
    if (str == NULL)
    {
        return string_empty();
    }

    // Already clean
    if (str->size == 0)
    {
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

    MALLOC_CHECK(string);

    string->data = data;
    string->size = strlen(data);

    return string;
}

void string_append(string_ptr string, char c)
{
    string->size = string->size + 1;
    string->data = realloc(string->data, string->size + 1); // null byte

    MALLOC_CHECK(string->data);

    string->data[string->size - 1] = c;
    string->data[string->size] = '\0';
}

// take last 3 characters from the buffer and turn them into a single symbol
void string_num_to_asci(string_ptr string, int base)
{
    char *num_str;

    if (base == 8)
    {
        num_str = malloc(sizeof(char) * 3);

        MALLOC_CHECK(num_str);

        // last 3 chars

        num_str[0] = string->data[string->size - 3];
        num_str[1] = string->data[string->size - 2];
        num_str[2] = string->data[string->size - 1];
        num_str[3] = '\0';
    }
    else if (base == 16)
    {
        // 0xAA
        num_str = malloc(sizeof(char) * 4);
        MALLOC_CHECK(num_str);

        // last 2 chars
        // ignore the x
        num_str[0] = string->data[string->size - 2];
        num_str[1] = string->data[string->size - 1];
        num_str[2] = '\0';
    }
    else
    {
        fprintf(stderr, "string_num_to_asci invalid base: %d\n", base);
        exit(99);
    }
    
    int dec = (int)strtol(num_str, NULL, base);

    string->size = string->size - 3;
    string->data = realloc(string->data, string->size + 1); // null byte

    MALLOC_CHECK(string->data);

    string->data[string->size - 1] = (char)dec;
    string->data[string->size] = '\0';

    free(num_str);
}

void string_print(string_ptr string)
{
    printf("Buffer (%d): %s\n", string->size, string->data);
}

// Create a new data array for the string, don't free the current data buffer.
void string_clean(string_ptr string)
{
    string->data = malloc(sizeof(char));

    MALLOC_CHECK(string->data);

    string->data[0] = '\0';
    string->size = 0;
}

void string_dispose(string_ptr string)
{
    free(string->data);
    free(string);
}