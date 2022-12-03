#include "buffer.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *alloc_str(const char *str) {
    char *s = malloc(sizeof(char) * (strlen(str) + 1));

    MALLOC_CHECK(s);

    strcpy(s, str);

    return s;
}

char *int_to_str(int i) {
    size_t len = snprintf(NULL, 0, "%d", i);

    char *s = malloc(sizeof(char) * len);

    MALLOC_CHECK(s);

    sprintf(s, "%d", i);

    return s;
}

char *float_to_str(double d) {
    size_t len = snprintf(NULL, 0, "%f", d);

    char *s = malloc(sizeof(char) * len);

    MALLOC_CHECK(s);

    sprintf(s, "%f", d);

    return s;
}

buffer_ptr buffer_init()
{
    buffer_ptr buffer = malloc(sizeof(struct buffer_t));

    MALLOC_CHECK(buffer);

    buffer->data = malloc(sizeof(char));

    MALLOC_CHECK(buffer->data);

    buffer->data[0] = '\0';
    buffer->size = 0;

    return buffer;
}

void buffer_append(buffer_ptr buffer, char c)
{
    buffer->size = buffer->size + 1;
    buffer->data = realloc(buffer->data, buffer->size + 1); // null byte

    MALLOC_CHECK(buffer->data);

    buffer->data[buffer->size - 1] = c;
    buffer->data[buffer->size] = '\0';
}

// take last 3 characters from the buffer and turn them into a single symbol
void buffer_num_to_asci(buffer_ptr buffer, int base)
{
    char *num_str;

    if (base == 8)
    {
        num_str = malloc(sizeof(char) * 3);

        MALLOC_CHECK(num_str);

        // last 3 chars

        num_str[0] = buffer->data[buffer->size - 3];
        num_str[1] = buffer->data[buffer->size - 2];
        num_str[2] = buffer->data[buffer->size - 1];
        num_str[3] = '\0';
    }
    else if (base == 16)
    {
        // 0xAA
        num_str = malloc(sizeof(char) * 4);
        MALLOC_CHECK(num_str);

        // last 2 chars
        // ignore the x
        num_str[0] = buffer->data[buffer->size - 2];
        num_str[1] = buffer->data[buffer->size - 1];
        num_str[2] = '\0';
    }
    else
    {
        fprintf(stderr, "buffer_num_to_asci invalid base: %d\n", base);
        exit(FAIL_INTERNAL);
    }

    int dec = (int)strtol(num_str, NULL, base);

    buffer->size = buffer->size - 3;
    buffer->data = realloc(buffer->data, buffer->size + 1); // null byte

    MALLOC_CHECK(buffer->data);

    buffer->data[buffer->size - 1] = (char)dec;
    buffer->data[buffer->size] = '\0';

    free(num_str);
}

void buffer_print(buffer_ptr buffer)
{
    printf("Buffer (%d): %s\n", buffer->size, buffer->data);
}

void buffer_reset(buffer_ptr buffer)
{
    free(buffer->data);

    buffer->data = malloc(sizeof(char));

    MALLOC_CHECK(buffer->data);

    buffer->data[0] = '\0';
    buffer->size = 0;
}

void buffer_dispose(buffer_ptr buffer)
{
    if (buffer == NULL)
    {
        return;
    }

    if (buffer->data != NULL)
    {
        free(buffer->data);
    }
    free(buffer);
}