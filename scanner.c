/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xbalek02 Miroslav Bálek
 * @author xdobes22 Kristán Dobeš
 * @author xsynak03 Maroš Synák
 */

#include "scanner.h"
#include "token.h"
#include "utils.h"
#include "buffer.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

bool attempt_keyword(stack_ptr stack, buffer_ptr buffer, char *keyword_str, keyword_t keyword)
{
    if (strcmp(buffer->data, keyword_str) == 0)
    {
        APPEND_KEYWORD(stack, keyword);
        buffer_reset(buffer);
        return true;
    }
    return false;
}

bool attempt_type(stack_ptr stack, buffer_ptr buffer, char *type_str, type_t type)
{
    if (strcmp(buffer->data, type_str) == 0)
    {
        APPEND_TYPE(stack, type);
        buffer_reset(buffer);
        return true;
    }
    return false;
}

bool attempt_greedy(char *rest, bool ignore_whitespace)
{
    DEBUG("GREEDY: %s", rest);

    // Found declare, be greedy and eat more characters.
    unsigned int i = 0; // chars read

    bool whole = true;

    char c = fgetc(stdin);

    buffer_ptr read = buffer_init();
    buffer_append(read, c);

    for (; i < strlen(rest);)
    {
        if (!isspace(c) || !ignore_whitespace)
        {
            if (rest[i] != c)
            {
                whole = false;
                break;
            }
            i++;
        }
        c = fgetc(stdin);
        buffer_append(read, c);
    }

    if (whole)
    {
        DEBUG("Ate it, nom.");
        return true;
    }
    else
    {
        for (int i = read->size - 1; i >= 0; i--)
        {
            ungetc(read->data[i], stdin);
        }
    }
    buffer_dispose(read);
    DEBUG("Not complete.");
    return false;
}

bool parse_character(stack_ptr stack, buffer_ptr buffer, int *scanner_state, char c, int line, int character)
{
    DEBUG("Scanning character: %c (%d)\n", c, c);

    switch (*scanner_state)
    {
    case (SCANNER_START):
    {
        if (c == '(')
        {
            APPEND_EMPTY(stack, TOKEN_L_PAREN);
        }
        else if (c == ')')
        {
            APPEND_EMPTY(stack, TOKEN_R_PAREN);
        }
        else if (c == '"')
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer_reset(buffer);
        }
        else if (c == '.')
        {
            APPEND_EMPTY(stack, TOKEN_DOT);
        }
        else if (c == ':')
        {
            APPEND_EMPTY(stack, TOKEN_COLON);
        }
        else if (c == '?')
        {
            CHANGE_STATE(SCANNER_NULLABLE);
        }
        else if (c == '}')
        {
            APPEND_EMPTY(stack, TOKEN_RC_BRACKET);
        }
        else if (c == '{')
        {
            APPEND_EMPTY(stack, TOKEN_LC_BRACKET);
        }
        else if (c == ',')
        {
            APPEND_EMPTY(stack, TOKEN_COMMA);
        }
        else if (c == ';')
        {
            APPEND_EMPTY(stack, TOKEN_SEMICOLON);
        }
        else if (c == '+')
        {
            APPEND_EMPTY(stack, TOKEN_PLUS);
        }
        else if (c == '-')
        {
            APPEND_EMPTY(stack, TOKEN_MINUS);
        }
        else if (c == '*')
        {
            APPEND_EMPTY(stack, TOKEN_MULTIPLE);
        }
        else if (c >= '0' && c <= '9')
        {
            CHANGE_STATE(SCANNER_NUM_INT);
            buffer_reset(buffer);
            buffer_append(buffer, c);
        }
        else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
        {
            CHANGE_STATE(SCANNER_ID);
            buffer_reset(buffer);
            buffer_append(buffer, c);
        }
        else if (c == '/')
        {
            CHANGE_STATE(SCANNER_DIVIDE);
        }
        else if (c == '>')
        {
            CHANGE_STATE(SCANNER_MORE_THAN);
        }
        else if (c == '<')
        {
            CHANGE_STATE(SCANNER_LESS_THAN);
        }
        else if (c == '!')
        {
            CHANGE_STATE(SCANNER_EXCL_MARK);
        }
        else if (c == '=')
        {
            CHANGE_STATE(SCANNER_ASIGN);
        }
        else if (c == '$')
        {
            CHANGE_STATE(SCANNER_VAR_ID_START);
            buffer_append(buffer, c);
        }
        else if (c == EOF || c == '\n' || c == ' ' || c == CR)
        {
            break;
        }
        else
        {
            fprintf(stderr, "Unexpected character (%c) (%d).\n", c, c);
            exit(FAIL_LEXICAL);
        }
        break;
    }
    case (SCANNER_NULLABLE):
    {
        if (c == '>')
        {
            CHANGE_STATE(SCANNER_EPILOG);
            APPEND_EMPTY(stack, TOKEN_CLOSING_TAG);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_EMPTY(stack, TOKEN_NULLABLE);

            parse_character(stack, buffer, scanner_state, c, line, character);
        }
        break;
    }
    case (SCANNER_NUM_INT):
    {
        if (c >= '0' && c <= '9')
        {
            buffer_append(buffer, c);
        }
        else if (c == '.')
        {
            CHANGE_STATE(SCANNER_NUM_POINT_START);
            buffer_append(buffer, c);
        }
        else if (c == 'E' || c == 'e')
        {
            CHANGE_STATE(SCANNER_NUM_EXP_START);
            buffer_append(buffer, c);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);

            // TODO: Parse better
            int val = atoi(buffer->data);

            APPEND_INT(stack, TOKEN_CONST_INT, val);

            buffer_reset(buffer);

            parse_character(stack, buffer, scanner_state, c, line, character);
        }
        break;
    }
    case (SCANNER_NUM_POINT_START):
    {
        if (c >= '0' && c <= '9')
        {
            CHANGE_STATE(SCANNER_NUM_DOUBLE);
            buffer_append(buffer, c);
        }
        else
        {
            fprintf(stderr, "Integer expected got (%c) instead. \n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        break;
    }
    case (SCANNER_NUM_DOUBLE):
    {
        if (c >= '0' && c <= '9')
        {
            buffer_append(buffer, c);
        }
        else if (c == 'E' || c == 'e')
        {
            CHANGE_STATE(SCANNER_NUM_EXP_START);
            buffer_append(buffer, c);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);

            double val = atof(buffer->data);

            APPEND_FLOAT(stack, TOKEN_CONST_DOUBLE, val);

            buffer_reset(buffer);

            parse_character(stack, buffer, scanner_state, c, line, character);
        }

        break;
    }
    case (SCANNER_NUM_EXP_START):
    {
        if (c >= '0' && c <= '9')
        {
            CHANGE_STATE(SCANNER_NUM_EXP);
            buffer_append(buffer, c);
        }
        else if (c >= '+' || c <= '-')
        {
            CHANGE_STATE(SCANNER_NUM_EXP_SIGN);
            buffer_append(buffer, c);
        }
        else
        {

            fprintf(stderr, "Expected integer or sign got (%c).\n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        break;
    }
    case (SCANNER_NUM_EXP_SIGN):
    {
        if (c >= '0' && c <= '9')
        {
            CHANGE_STATE(SCANNER_NUM_EXP);
            buffer_append(buffer, c);
        }
        else
        {

            fprintf(stderr, "Expected integer got (%c) instead.\n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        break;
    }
    case (SCANNER_NUM_EXP):
    {
        if (c >= '0' && c <= '9')
        {
            buffer_append(buffer, c);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);

            // TODO: Parse better
            double val = atof(buffer->data);

            APPEND_FLOAT(stack, TOKEN_CONST_DOUBLE, val);

            buffer_reset(buffer);

            parse_character(stack, buffer, scanner_state, c, line, character);
        }

        break;
    }
    case (SCANNER_STRING):
    {
        if (c == '"')
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_STRING(stack, TOKEN_STRING_LIT, buffer);
        }
        else if (c == '\\')
        {
            CHANGE_STATE(SCANNER_STRING_ESCAPE);
            buffer_append(buffer, c);
        }
        else if (c == '$')
        {
            // TODO: correct error code
            exit(FAIL_LEXICAL);
        }
        else
        {
            buffer_append(buffer, c);
        }
        break;
    }
    case (SCANNER_STRING_ESCAPE):
    {
        if (c == 'x')
        {
            CHANGE_STATE(SCANNER_HEX_START);
            buffer_append(buffer, c);
        }
        else if (c >= '0' && c <= '3')
        {
            CHANGE_STATE(SCANNER_OCTA_1);
            buffer_append(buffer, c);
        }
        else if (c == '"')
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer->data[buffer->size - 1] = '"';
        }
        else if (c == '\\')
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer->data[buffer->size - 1] = '\\';
        }
        else if (c == '$')
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer->data[buffer->size - 1] = '$';
        }
        else if (c == 'n')
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer->data[buffer->size - 1] = '\n';
        }
        else if (c == 'r')
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer->data[buffer->size - 1] = '\r';
        }
        else if (c == 't')
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer->data[buffer->size - 1] = '\t';
        }
        else
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer_append(buffer, c);
        }
        break;
    }
    case (SCANNER_HEX_START):
    {
        if (c == '"')
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_STRING(stack, TOKEN_STRING_LIT, buffer);
        }
        else if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9'))
        {
            CHANGE_STATE(SCANNER_HEX_FIRST);
            buffer_append(buffer, c);
        }
        else if (c == '$')
        {
            fprintf(stderr, "Symbol (%c) cannot be used unescaped. \n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        else
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer_append(buffer, c);
        }

        break;
    }
    case (SCANNER_HEX_FIRST):
    {
        if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9'))
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer_append(buffer, c);
            buffer_num_to_asci(buffer, 16);
        }
        else if (c == '"')
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_STRING(stack, TOKEN_STRING_LIT, buffer);
        }
        else if (c == '$')
        {
            fprintf(stderr, "Symbol (%c) cannot be used unescaped. \n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        else
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer_append(buffer, c);
        }
        break;
    }
    case (SCANNER_OCTA_1):
    {
        if (c == '"')
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_STRING(stack, TOKEN_STRING_LIT, buffer);
        }
        else if ((c >= '0' && c <= '7'))
        {
            CHANGE_STATE(SCANNER_OCTA_2);
            buffer_append(buffer, c);
        }
        else if (c == '$')
        {
            fprintf(stderr, "Symbol (%c) cannot be used unescaped. \n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        else
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer_append(buffer, c);
        }
        break;
    }
    case (SCANNER_OCTA_2):
    {
        if (c == '"')
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_STRING(stack, TOKEN_STRING_LIT, buffer);
        }
        else if ((c >= '0' && c <= '7'))
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer_append(buffer, c);
            buffer_num_to_asci(buffer, 8);
        }
        else if (c == '$')
        {
            fprintf(stderr, "Symbol (%c) cannot be used unescaped. \n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        else
        {
            CHANGE_STATE(SCANNER_STRING);
            buffer_append(buffer, c);
        }
        break;
    }
    case (SCANNER_MORE_THAN):
    {
        if (c == '=')
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_EMPTY(stack, TOKEN_MORE_EQUAL);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_EMPTY(stack, TOKEN_MORE);

            parse_character(stack, buffer, scanner_state, c, line, character);
        }
        break;
    }
    case (SCANNER_LESS_THAN):
    {
        CHANGE_STATE(SCANNER_START);

        if (c == '=')
        {
            APPEND_EMPTY(stack, TOKEN_LESS_EQUAL);
        }
        else if (c == '?')
        {
            if (attempt_greedy("php", false))
            {
                APPEND_EMPTY(stack, TOKEN_OPENING_TAG);
            }
        }
        else
        {
            APPEND_EMPTY(stack, TOKEN_LESS);
            parse_character(stack, buffer, scanner_state, c, line, character);
        }
        break;
    }
    case (SCANNER_ASIGN):
    {
        if (c == '=')
        {
            CHANGE_STATE(SCANNER_EQ_START);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_EMPTY(stack, TOKEN_ASSIGN);

            parse_character(stack, buffer, scanner_state, c, line, character);
        }
        break;
    }
    case (SCANNER_EQ_START):
    {
        if (c == '=')
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_EMPTY(stack, TOKEN_EQUAL);
        }
        else
        {
            fprintf(stderr, "Equality is compared via \"===\". \n");
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        break;
    }
    case (SCANNER_EXCL_MARK):
    {
        if (c == '=')
        {
            CHANGE_STATE(SCANNER_NOT_EQ_START);
        }
        else
        {
            fprintf(stderr, "Expected (=) got (%c) instead.\n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        break;
    }
    case (SCANNER_NOT_EQ_START):
    {
        if (c == '=')
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_EMPTY(stack, TOKEN_NOT_EQUAL);
        }
        else
        {
            fprintf(stderr, "Expected (=) got (%c) instead.\n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }

        break;
    }
    case (SCANNER_DIVIDE):
    {
        if (c == '/')
        {
            CHANGE_STATE(SCANNER_LINE_COMM);
        }
        else if (c == '*')
        {
            CHANGE_STATE(SCANNER_BLOCK_COMM);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_EMPTY(stack, TOKEN_DIVIDE);

            parse_character(stack, buffer, scanner_state, c, line, character);
        }
        break;
    }
    case (SCANNER_LINE_COMM):
    {
        if (c == EOF || c == '\n' || c == CR)
        {
            CHANGE_STATE(SCANNER_START);
        }
        break;
    }
    case (SCANNER_BLOCK_COMM):
    {
        if (c == '*')
        {
            CHANGE_STATE(SCANNER_BLOCK_END);
        }
        break;
    }
    case (SCANNER_BLOCK_END):
    {
        if (c == '/')
        {
            CHANGE_STATE(SCANNER_START);
        }
        else if (c != '*')
        {
            CHANGE_STATE(SCANNER_BLOCK_COMM);
        }
        break;
    }
    case (SCANNER_ID):
    {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (c >= '0' && c <= '9'))
        {
            buffer_append(buffer, c);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);

            ungetc(c, stdin);
            if (strcmp("declare", buffer->data) == 0 && attempt_greedy("(strict_types=1);", true))
            {
                APPEND_EMPTY(stack, TOKEN_DECLARE);
                buffer_reset(buffer);
                break;
            }
            else
            {
                c = fgetc(stdin);
            }

            // check if the buffer content matches any keywords / types
            if (!attempt_keyword(stack, buffer, "function", KEYWORD_FUNCTION) &&
                !attempt_keyword(stack, buffer, "if", KEYWORD_IF) &&
                !attempt_keyword(stack, buffer, "else", KEYWORD_ELSE) &&
                !attempt_keyword(stack, buffer, "while", KEYWORD_WHILE) &&
                !attempt_keyword(stack, buffer, "return", KEYWORD_RETURN) &&
                !attempt_type(stack, buffer, "int", TYPE_INT) &&
                !attempt_type(stack, buffer, "string", TYPE_STRING) &&
                !attempt_type(stack, buffer, "float", TYPE_FLOAT) &&
                !attempt_type(stack, buffer, "void", TYPE_VOID))
            {
                // none of the keywords matched, create a token id from this
                APPEND_STRING(stack, TOKEN_ID, buffer);
                parse_character(stack, buffer, scanner_state, c, line, character);
            }
            else
            {
                parse_character(stack, buffer, scanner_state, c, line, character);
            }
        }
        break;
    }
    case (SCANNER_VAR_ID_START):
    {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
        {
            CHANGE_STATE(SCANNER_VAR_ID);
            buffer_append(buffer, c);
        }
        else
        {
            fprintf(stderr, "Variables have to start with alphanumeric chatacters. (%c)\n", c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        break;
    }
    case (SCANNER_VAR_ID):
    {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (c >= '0' && c <= '9'))
        {
            buffer_append(buffer, c);
        }
        else
        {
            CHANGE_STATE(SCANNER_START);
            APPEND_STRING(stack, TOKEN_VAR_ID, buffer);

            parse_character(stack, buffer, scanner_state, c, line, character);
        }
        break;
    }
    case (SCANNER_EPILOG):
    {
        if (c != EOF && c != '\n' && c != CR)
        {
            fprintf(stderr, "Character %d (%c) after epilog.\n", c, c);
            fprintf(stderr, "Error on line %d character %d \n", line, character);
            exit(FAIL_LEXICAL);
        }
        break;
    }
    default:
        fprintf(stderr, "Unknown scanner state %d.\n", *scanner_state);
        exit(FAIL_LEXICAL);
    }
    return c == EOF;
}

stack_ptr tokenize()
{
    int c;

    int scanner_state = SCANNER_START;

    // Buffer for parses that require multiple char reads
    buffer_ptr buffer = buffer_init();

    stack_ptr stack = stack_init();
    int line = 1;
    int character = 1;
    while (!parse_character(stack, buffer, &scanner_state, (c = fgetc(stdin)), line, character))
    {
        // Line count may not optimalized for ios
        if (c == '\n')
        {
            line++;
            character = 1;
        }
        else
        {
            character++;
        }
    }

    // scanner cannot end on anything else than start and epilog
    // means we started parsing something and didn't finish
    if (scanner_state != SCANNER_START && scanner_state != SCANNER_EPILOG)
    {
        DEBUG_STACK(stack);
        fprintf(stderr, "Didn't finish scanning. Left on state: %d\n", scanner_state);
        fprintf(stderr, "Error on line %d character %d \n", line, character);
        exit(FAIL_LEXICAL);
    }

    // Dispose of buffer
    buffer_dispose(buffer);
    return stack;
}
