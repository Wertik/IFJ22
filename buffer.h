#ifndef _BUFFER_H
#define _BUFFER_H

typedef struct buffer_t
{
    // String size without null byte
    // (means that the data array is always one byte bigger than size says)
    int size;
    char *data;
} * buffer_ptr;

// Initialize an empty buffer
buffer_ptr buffer_init();

// Append a character to buffer
void buffer_append(buffer_ptr buffer, char c);

// Print the buffer
void buffer_print(buffer_ptr buffer);

// Convert last 3 characters on the buffer to a single char using octa / hex ascii values.
void buffer_num_to_asci(buffer_ptr buffer, int format);

// Reset the buffer
void buffer_reset(buffer_ptr buffer);

// Dispose of the buffer
void buffer_dispose(buffer_ptr buffer);

#endif