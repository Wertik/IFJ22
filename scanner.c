#include "scanner.h"
#include "token.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool parse_character(array_ptr tokens, string_ptr *buffer, int *scanner_state, char c)
{
    switch (*scanner_state)
    {
    case (SCANNER_START):
    {
        if (c == '(')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_L_PAREN, NONE, value);
            array_append(tokens, token);
        }
        else if (c == ')')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_R_PAREN, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '"')
        {
            *scanner_state = SCANNER_STRING;

            // create a fresh buffer
            *buffer = string_empty();
        }
        break;
    }
    case (SCANNER_STRING):
    {
        if (c == '"')
        {
            *scanner_state = SCANNER_START;

            token_value_t value = {.string = (*buffer)->data};
            token_ptr token = token_create(TOKEN_STRING_LIT, STRING, value);
            array_append(tokens, token);

            // free the string struct, but keep the data array alive
            string_destroy(*buffer);
        }
        else
        {
            string_append(*buffer, c);
        }
        break;
    }
    default:
        fprintf(stderr, "Unknown scanner state %d.\n", *scanner_state);
        exit(1);
    }
    return false;
}

void tokenize(array_ptr tokens)
{
    int c;

    /* Keep a count of how many lines we read to display pretty semantic errors. */
    int line_count = 0;
    /* Keep a count of how many characters we read in this line to display pretty semantic errors. */
    int character_count = 0;

    int scanner_state = SCANNER_START;

    // Buffer for parses that require multiple char reads
    string_ptr buffer;

    while (true)
    {
        c = getc(stdin);

        if ((c == (int)'\n' || c == EOF) && character_count != 0) // ignore empty lines
        {
            line_count++;
            character_count = 0;
        }
        else
        {
            character_count++;
        }

        if (c == EOF || parse_character(tokens, &buffer, &scanner_state, (char)c))
        {
            break;
        }
    }
}
