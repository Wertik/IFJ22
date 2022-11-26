#include "scanner.h"
#include "token.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool attempt_keyword(array_ptr tokens, string_ptr *buffer, char *keyword_str, keyword_t keyword)
{
    if (strcmp((*buffer)->data, keyword_str) == 0)
    {
        append_keyword(tokens, keyword);
        string_fresh(*buffer);
        return true;
    }
    return false;
}

bool attempt_type(array_ptr tokens, string_ptr *buffer, char *type_str, type_t type)
{
    if (strcmp((*buffer)->data, type_str) == 0)
    {
        append_type(tokens, type);
        string_fresh(*buffer);
        return true;
    }
    return false;
}

void append_token(array_ptr tokens, token_type_t token_type)
{
    token_value_t value;
    token_ptr token = token_create(token_type, NONE, value);
    array_append(tokens, token);
}

void append_keyword(array_ptr tokens, keyword_t keyword)
{
    token_value_t value = {.keyword = keyword};

    token_ptr token = token_create(TOKEN_KEYWORD, KEYWORD, value);
    array_append(tokens, token);
}

void append_type(array_ptr tokens, type_t type)
{
    token_value_t value = {.type = type};

    token_ptr token = token_create(TOKEN_TYPE, TYPE, value);
    array_append(tokens, token);
}

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
            *scanner_state = SCANNER_NULLABLE;
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
        break;
    }
    case (SCANNER_NULLABLE):
    {
        if (c == '>')
        {
            *scanner_state = SCANNER_START;

            append_token(tokens, TOKEN_CLOSING_TAG);
        }
        else
        {
            *scanner_state = SCANNER_START;

            append_token(tokens, TOKEN_NULLABLE);

            parse_character(tokens, buffer, scanner_state, c);
        }
        break;
    }
    case (SCANNER_NUM_INT):
    {
        if (c >= '0' && c <= '9')
        {
            string_append(*buffer, c);
        }
        else if (c == '.')
        {
            *scanner_state = SCANNER_NUM_POINT_START;
            string_append(*buffer, c);
        }
        else if (c == 'E' || c == 'e')
        {
            *scanner_state = SCANNER_NUM_EXP_START;
            string_append(*buffer, c);
        }
        else
        {
            *scanner_state = SCANNER_START;

            // TODO: Parse better
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
    case (SCANNER_NUM_POINT_START):
    {
        if (c >= '0' && c <= '9')
        {
            *scanner_state = SCANNER_NUM_DOUBLE;
            string_append(*buffer, c);
        }
        else
        {
            // TODO: do better
            fprintf(stderr, "Syntax error. \n", c);
            exit(1);
        }
        break;
    }
    case (SCANNER_NUM_DOUBLE):
    {
        if (c >= '0' && c <= '9')
        {
            string_append(*buffer, c);
        }
        else if (c == 'E' || c == 'e')
        {
            *scanner_state = SCANNER_NUM_EXP_START;
            string_append(*buffer, c);
        }
        else
        {
            *scanner_state = SCANNER_START;

            // TODO: Parse better
            double val = atof((*buffer)->data);

            token_value_t value = {.float_value = val};
            token_ptr token = token_create(TOKEN_CONST_DOUBLE, DOUBLE, value);
            array_append(tokens, token);

            // free the string struct, but keep the data array alive
            string_fresh(*buffer);

            parse_character(tokens, buffer, scanner_state, c);
        }

        break;
    }
    case (SCANNER_NUM_EXP_START):
    {
        if (c >= '0' && c <= '9')
        {
            *scanner_state = SCANNER_NUM_EXP;
            string_append(*buffer, c);
        }
        else if (c >= '+' || c <= '-')
        {
            *scanner_state = SCANNER_NUM_EXP_SIGN;
            string_append(*buffer, c);
        }
        else
        {
            // TODO: do better
            fprintf(stderr, "Syntax error. \n", c);
            exit(1);
        }
        break;
    }
    case (SCANNER_NUM_EXP_SIGN):
    {
        if (c >= '0' && c <= '9')
        {
            *scanner_state = SCANNER_NUM_EXP;
            string_append(*buffer, c);
        }else
        {
            //TODO: do better 
            fprintf(stderr, "Syntax error. \n", c);
            exit(1);
        }
        break;
    }
    case (SCANNER_NUM_EXP):
    {
        if (c >= '0' && c <= '9')
        {
            string_append(*buffer, c);
        }
        else
        {
            *scanner_state = SCANNER_START;

            // TODO: Parse better
            double val = atof((*buffer)->data);

            token_value_t value = {.float_value = val};
            token_ptr token = token_create(TOKEN_CONST_DOUBLE, DOUBLE, value);
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
        else if (c == '\\')
        {
            *scanner_state = SCANNER_STRING_ESCAPE;
            string_append(*buffer, c);
        }
        else if (c == '$')
        {
            // TODO: correct error code
            exit(1);
        }
        else
        {
            string_append(*buffer, c);
        }
        break;
    }
    case (SCANNER_STRING_ESCAPE):
    {
        if (c == 'x')
        {
            *scanner_state = SCANNER_HEX_START;
            string_append(*buffer, c);
        }
        else if (c >= '0' && c <= '3')
        {
            *scanner_state = SCANNER_OCTA_1;
            string_append(*buffer, c);
        }
        else if (c == '"')
        {
            // TODO:
        }
        else if (c == '\\')
        {
            // TODO:
        }
        else if (c == '$')
        {
            // TODO:
        }
        else if (c == 'n')
        {
            (*buffer)->data[(*buffer)->size - 1] = '\n';
        }
        else if (c == 't')
        {
            (*buffer)->data[(*buffer)->size - 1] = '\t';
        }

        else
        {
            *scanner_state = SCANNER_STRING;
            string_append(*buffer, c);
        }

        break;
    }
    case (SCANNER_HEX_START):
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
        else if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9'))
        {
            *scanner_state = SCANNER_HEX_FIRST;
            string_append(*buffer, c);
        }
        else if (c == '$')
        {
            fprintf(stderr, "Symbol (%c) cannot be used unescaped. \n", c);
            exit(1);
        }
        else
        {
            *scanner_state = SCANNER_STRING;
            string_append(*buffer, c);
        }

        break;
    }
    case (SCANNER_HEX_FIRST):
    {
        if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9'))
        {
            *scanner_state = SCANNER_STRING;
            string_append(*buffer, c);
            string_num_to_asci(*buffer, 16);
        }
        else if (c == '"')
        {
            *scanner_state = SCANNER_START;

            token_value_t value = {.string = (*buffer)->data};
            token_ptr token = token_create(TOKEN_STRING_LIT, STRING, value);
            array_append(tokens, token);

            // free the string struct, but keep the data array alive
            string_clean(*buffer);
        }
        else if (c == '$')
        {
            fprintf(stderr, "Symbol (%c) cannot be used unescaped. \n", c);
            exit(1);
        }
        else
        {
            *scanner_state = SCANNER_STRING;
            string_append(*buffer, c);
        }

        break;
    }
    case (SCANNER_OCTA_1):
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
        else if ((c >= '0' && c <= '7'))
        {
            *scanner_state = SCANNER_OCTA_2;
            string_append(*buffer, c);
        }
        else if (c == '$')
        {
            fprintf(stderr, "Symbol (%c) cannot be used unescaped. \n", c);
            exit(1);
        }
        else
        {
            *scanner_state = SCANNER_STRING;
            string_append(*buffer, c);
        }
        break;
    }
    case (SCANNER_OCTA_2):
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
        else if ((c >= '0' && c <= '7'))
        {
            *scanner_state = SCANNER_STRING;
            string_append(*buffer, c);
            string_num_to_asci(*buffer, 8);
        }
        else if (c == '$')
        {
            fprintf(stderr, "Symbol (%c) cannot be used unescaped. \n", c);
            exit(1);
        }
        else
        {
            *scanner_state = SCANNER_STRING;
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
        *scanner_state = SCANNER_START;

        if (c == '=')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_LESS_EQUAL, NONE, value);
            array_append(tokens, token);
        }
        else if (c == '?')
        {
            token_value_t value;
            token_ptr token = token_create(TOKEN_OPENING_TAG, NONE, value);
            array_append(tokens, token);
        }
        else
        {
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
        else
        {
            fprintf(stderr, "Equality is compared via \"===\". \n", c);
            exit(1);
        }

        break;
    }
    case (SCANNER_EXCL_MARK):
    {
        if (c == '=')
        {
            *scanner_state = SCANNER_NOT_EQ_START;
        }
        else
        {
            // TODO: do better
            fprintf(stderr, "Syntax error. \n", c);
            exit(1);
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
        else
        {
            fprintf(stderr, "Equality is compared via \"===\". \n", c);
            exit(1);
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
        if (c == EOF || c == '\n')
        {
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

            if (!attempt_keyword(tokens, buffer, "function", KEYWORD_FUNCTION) &&
                !attempt_keyword(tokens, buffer, "if", KEYWORD_IF) &&
                !attempt_keyword(tokens, buffer, "else", KEYWORD_ELSE) &&
                !attempt_keyword(tokens, buffer, "while", KEYWORD_WHILE) &&
                !attempt_keyword(tokens, buffer, "return", KEYWORD_RETURN) &&
                !attempt_type(tokens, buffer, "int", TYPE_INT) &&
                !attempt_type(tokens, buffer, "string", TYPE_STRING) &&
                !attempt_type(tokens, buffer, "float", TYPE_FLOAT) &&
                !attempt_type(tokens, buffer, "void", TYPE_VOID))
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
            fprintf(stderr, "Variables have to start with alphanumeric chatacters. (%c)\n", c);
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

            parse_character(tokens, buffer, scanner_state, c);
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
