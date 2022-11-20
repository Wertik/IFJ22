#include "scanner.h"
#include "token.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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
            *buffer = string_fresh(*buffer);
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
            token_ptr token = token_create(TOKEN_NULLABLE, NONE, value);
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
        else if (c >= '0' && c <= '9')
        {
            *scanner_state = SCANNER_NUM_INT;

            // create a fresh buffer
            *buffer = string_fresh(*buffer);
            string_append(*buffer, c);
        }
        else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
        {
            *scanner_state = SCANNER_ID;

            *buffer = string_fresh(*buffer);
            string_append(*buffer, c);
        }
        else if (c == '/')
        {
            *scanner_state = SCANNER_DIVIDE;
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
        else if (c == ';')
        {
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_SEMICOLON, NONE, value);
            array_append(tokens, token);
        }
        break;
    }
    case (SCANNER_NUM_INT):
    {
        if (c >= '0' && c <= '9')
        {
            string_append(*buffer, c);
        }
        else
        {
            *scanner_state = SCANNER_START;

            int val = atoi((*buffer)->data);

            token_value_t value = {.integer = val};
            token_ptr token = token_create(TOKEN_CONST_INT, INTEGER, value);
            array_append(tokens, token);

            // free the string struct, but keep the data array alive
            string_fresh(*buffer);

            parse_character(tokens, buffer, scanner_state, c);
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
            string_clean(*buffer);
        }
        else
        {
            string_append(*buffer, c);
        }
        break;
    }
    case (SCANNER_MORE_THAN):
    {
        if (c == '=')
        {

            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_MORE_EQUAL, NONE, value);
            array_append(tokens, token);
        }
        else
        {
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_MORE, NONE, value);
            array_append(tokens, token);

            parse_character(tokens, buffer, scanner_state, (char)c);
        }
        break;
    }
    case (SCANNER_LESS_THAN):
    {
        if (c == '=')
        {

            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_LESS_EQUAL, NONE, value);
            array_append(tokens, token);
        }
        else
        {
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_LESS, NONE, value);
            array_append(tokens, token);

            parse_character(tokens, buffer, scanner_state, (char)c);
        }
        break;
    }
    case (SCANNER_ASIGN):
    {
        if (c == '=')
        {
            *scanner_state = SCANNER_EQ_START;
        }
        else
        {

            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_ASSIGN, NONE, value);
            array_append(tokens, token);

            parse_character(tokens, buffer, scanner_state, (char)c);
        }
        break;
    }
    case (SCANNER_EQ_START):
    {
        if (c == '=')
        {
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_EQUAL, NONE, value);
            array_append(tokens, token);
        }
        break;
    }
    case (SCANNER_EXCL_MARK):
    {
        if (c == '=')
        {
            *scanner_state = SCANNER_NOT_EQ_START;
        }
        break;
    }
    case (SCANNER_NOT_EQ_START):
    {
        if (c == '=')
        {
            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_NOT_EQUAL, NONE, value);
            array_append(tokens, token);
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

            *scanner_state = SCANNER_START;

            token_value_t value;
            token_ptr token = token_create(TOKEN_DIVIDE, NONE, value);
            array_append(tokens, token);

            parse_character(tokens, buffer, scanner_state, (char)c);
        }
        break;
    }
    case (SCANNER_LINE_COMM):
    {
        if (!(c == EOF || c == '\n'))
        {
            *scanner_state = SCANNER_LINE_COMM;
        }else if(c == EOF || c == '\n'){
            *scanner_state = SCANNER_START;
        }
        break;
    }
    case (SCANNER_BLOCK_COMM):
    {
        if (c == '*')
        {
            *scanner_state = SCANNER_BLOCK_END;
        }
        break;
    }
    case (SCANNER_BLOCK_END):
    {
        if (c == '/')
        {
            *scanner_state = SCANNER_START;
        }
        else if (c != '*')
        {
            *scanner_state = SCANNER_BLOCK_COMM;
        }
        break;
    }
    case (SCANNER_ID):
    {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (c >= '0' && c <= '9'))
        {
            string_append(*buffer, c);
        }
        else
        {
            *scanner_state = SCANNER_START;

            if (strcmp((*buffer)->data, "function") == 0)
            {
                token_value_t value = {.keyword = KEYWORD_FUNCTION};

                token_ptr token = token_create(TOKEN_KEYWORD, KEYWORD, value);
                array_append(tokens, token);

                string_fresh(*buffer);
            }
            else if (strcmp((*buffer)->data, "if") == 0)
            {
                token_value_t value = {.keyword = KEYWORD_IF};

                token_ptr token = token_create(TOKEN_KEYWORD, KEYWORD, value);
                array_append(tokens, token);

                string_fresh(*buffer);
            }
            else if (strcmp((*buffer)->data, "else") == 0)
            {
                token_value_t value = {.keyword = KEYWORD_ELSE};

                token_ptr token = token_create(TOKEN_KEYWORD, KEYWORD, value);
                array_append(tokens, token);

                string_fresh(*buffer);
            }
            else if (strcmp((*buffer)->data, "return") == 0)
            {
                token_value_t value = {.keyword = KEYWORD_RETURN};

                token_ptr token = token_create(TOKEN_KEYWORD, KEYWORD, value);
                array_append(tokens, token);

                string_fresh(*buffer);
            }
            else
            {
                token_value_t value = {.string = (*buffer)->data};

                token_ptr token = token_create(TOKEN_ID, STRING, value);
                array_append(tokens, token);

                // free the string struct, but keep the data array alive
                string_clean(*buffer);
            }

            parse_character(tokens, buffer, scanner_state, (char)c);
        }
        break;
    }
    case (SCANNER_VAR_ID_START):
    {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
        {
            *scanner_state = SCANNER_VAR_ID;
            string_append(*buffer, c);
        }
        else
        {
            fprintf(stderr, "Variables have to start with alphanumeric chatacters. (%c)", c);
            exit(1);
        }
        break;
    }
    case (SCANNER_VAR_ID):
    {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (c >= '0' && c <= '9'))
        {
            string_append(*buffer, c);
        }
        else
        {
            *scanner_state = SCANNER_START;

            token_value_t value = {.string = (*buffer)->data};
            token_ptr token = token_create(TOKEN_VAR_ID, STRING, value);
            array_append(tokens, token);

            string_clean(*buffer);
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
    string_ptr buffer = NULL;

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

    string_dispose(buffer);
}
