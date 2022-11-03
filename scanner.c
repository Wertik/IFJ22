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
        else if (c == '.')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_DOT, NONE, value);
            array_append(tokens, token);
        }
        else if (c == ':')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_COLON, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '?')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_NULLABLE_START, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '}')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_RC_BRACKET, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '{')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_LC_BRACKET, NONE, value);
            array_append(tokens, token);
        }
        else if (c == ',')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_COMMA, NONE, value);
            array_append(tokens, token);
        }
        else if (c == ';')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_SEMICOLON, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '+')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_PLUS, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '-')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_MINUS, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '*')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_MULTIPLE, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '>')
        {
            *scanner_state = SCANNER_MORE_THAN;
        }
        else if (c == '<')
        {
            *scanner_state = SCANNER_LESS_THAN;
        }
        else if (c == '!')
        {
            *scanner_state = SCANNER_EXCL_MARK;
        }
        else if (c == '=')
        {
            *scanner_state = SCANNER_ASIGN;
        }
        else if (c == '$')
        {
            *scanner_state = SCANNER_VAR_ID_START;
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
    case (SCANNER_MORE_THAN):
    {
        if (c == "=")
        {

            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_MORE_EQUAL, NONE, value);
            array_append(tokens, token);
        }
        else
        {
            //?? what to do with char c?
            //!! lost c value here
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_MORE, NONE, value);
            array_append(tokens, token);
        }
        break;
    }
    case (SCANNER_LESS_THAN):
    {
        if (c == "=")
        {

            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_LESS_EQUAL, NONE, value);
            array_append(tokens, token);
        }
        else
        {
            //?? what to do with char c?
            //!! lost c value here
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_LESS, NONE, value);
            array_append(tokens, token);
        }
        break;
    }
    case (SCANNER_ASIGN):
    {
        if (c == "=")
        {

            *scanner_state = SCANNER_NOT_EQ_START;
        }
        else
        {
            //?? what to do with char c?
            //!! lost c value here
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_LESS, NONE, value);
            array_append(tokens, token);
        }
        break;
    }
    case (SCANNER_EXCL_MARK):
    {
        if (c == "=")
        {

            *scanner_state = SCANNER_EQ_START;
        }
        else
        {
            // TODO: error handling
        }
        break;
    }
    case (SCANNER_DIVIDE):
    {
        if (c == '/')
        {
            *scanner_state = SCANNER_LINE_COMM;
        }
        else if (c == '*')
        {
            *scanner_state = SCANNER_BLOCK_COMM;
        }
        else
        {
              //?? what to do with char c?
            //!! lost c value here
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_DIVIDE, NONE, value);
            array_append(tokens, token);
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
