#ifndef STRING_H
#define STRING_H

typedef struct string_t
{
    // String size without null byte
    // (means that the data array is always one byte bigger than size says)
    int size;
    char *data;
} * string_ptr;

string_ptr string_create(char *data);

string_ptr string_empty();

void string_append(string_ptr string, char c);

void string_print(string_ptr string);

void string_destroy(string_ptr string);

void string_dispose(string_ptr string);

#endif