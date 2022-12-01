#include "utils.h"
#include "token.h"
#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

token_ptr token_create(token_type_t type, token_value_type_t value_type, token_value_t value)
{
    token_ptr token = malloc(sizeof(struct token_t));

    MALLOC_CHECK(token);

    token->type = type;
    token->value_type = value_type;
    token->value = value;

    return token;
}

token_ptr token_create_empty(token_type_t type)
{
    token_value_t value;
    return token_create(type, NONE, value);
}

// Create a token with str as string value. Copies str into a new string, can be freed.
token_ptr token_create_string(token_type_t type, char *str)
{
    size_t len = strlen(str);

    char *token_str = malloc(sizeof(char) * (len + 1));

    MALLOC_CHECK(token_str);

    strcpy(token_str, str);

    token_value_t value = {.string = token_str};

    return token_create(type, STRING, value);
}

token_ptr token_create_keyword(keyword_t keyword)
{
    token_value_t value = {.keyword = keyword};
    return token_create(TOKEN_KEYWORD, KEYWORD, value);
}

token_ptr token_create_type(type_t type)
{
    token_value_t value = {.type = type};
    return token_create(TOKEN_TYPE, TYPE, value);
}

void token_dispose(token_ptr token)
{
    // free string values
    if (token->value_type == STRING)
    {
        free(token->value.string);
    }

    free(token);
}

// Parse type name to string for pretty print
char *token_type_to_name(token_type_t type)
{
    switch (type)
    {
    case TOKEN_CONST_INT:
        return "TOKEN_CONST_INT";
    case TOKEN_L_PAREN:
        return "TOKEN_L_PAREN";
    case TOKEN_R_PAREN:
        return "TOKEN_R_PAREN";
    case TOKEN_STRING_LIT:
        return "TOKEN_STRING_LIT";
    case TOKEN_RC_BRACKET:
        return "TOKEN_RC_BRACKET";
    case TOKEN_LC_BRACKET:
        return "TOKEN_LC_BRACKET";
    case TOKEN_NULLABLE:
        return "TOKEN_NULLABLE";
    case TOKEN_COLON:
        return "TOKEN_COLON";
    case TOKEN_DOT:
        return "TOKEN_DOT";
    case TOKEN_VAR_ID:
        return "TOKEN_VAR_ID";
    case TOKEN_COMMA:
        return "TOKEN_COMMA";
    case TOKEN_SEMICOLON:
        return "TOKEN_SEMICOLON";
    case TOKEN_PLUS:
        return "TOKEN_PLUS";
    case TOKEN_MINUS:
        return "TOKEN_MINUS";
    case TOKEN_MULTIPLE:
        return "TOKEN_MULTIPLE";
    case TOKEN_DIVIDE:
        return "TOKEN_DIVIDE";
    case TOKEN_MORE:
        return "TOKEN_MORE";
    case TOKEN_MORE_EQUAL:
        return "TOKEN_MORE_EQUAL";
    case TOKEN_LESS:
        return "TOKEN_LESS";
    case TOKEN_LESS_EQUAL:
        return "TOKEN_LESS_EQUAL";
    case TOKEN_CONST_DOUBLE:
        return "TOKEN_CONST_DOUBLE";
    case TOKEN_EQUAL:
        return "TOKEN_EQUAL";
    case TOKEN_ASSIGN:
        return "TOKEN_ASSIGN";
    case TOKEN_NOT_EQUAL:
        return "TOKEN_NOT_EQUAL";
    case TOKEN_CONST_EXP:
        return "TOKEN_CONST_EXP";
    case TOKEN_ID:
        return "TOKEN_ID";
    case TOKEN_KEYWORD:
        return "TOKEN_KEYWORD";
    case TOKEN_TYPE:
        return "TOKEN_TYPE";
    case TOKEN_OPENING_TAG:
        return "TOKEN_OPENING_TAG";
    case TOKEN_CLOSING_TAG:
        return "TOKEN_CLOSING_TAG";
    case TOKEN_DECLARE:
        return "TOKEN_DECLARE";
    default:
    {
        char *s = malloc(sizeof(char));
        sprintf(s, "%d", type);
        return s;
    }
    }
}

char *type_to_name(type_t type)
{
    switch (type)
    {
    case TYPE_INT:
        return "TYPE_INT";
    case TYPE_STRING:
        return "TYPE_STRING";
    case TYPE_FLOAT:
        return "TYPE_FLOAT";
    case TYPE_VOID:
        return "TYPE_VOID";
    default:
        return "#unknown-type";
    }
}

char *keyword_to_name(keyword_t keyword)
{
    switch (keyword)
    {
    case KEYWORD_IF:
        return "KEYWORD_IF";
    case KEYWORD_ELSE:
        return "KEYWORD_ELSE";
    case KEYWORD_WHILE:
        return "KEYWORD_WHILE";
    case KEYWORD_FUNCTION:
        return "KEYWORD_FUNCTION";
    case KEYWORD_RETURN:
        return "KEYWORD_RETURN";
    default:
        return "#unknown-keyword";
    }
}

// Parse value type name to string for pretty print
char *value_type_to_name(token_value_type_t value_type)
{
    switch (value_type)
    {
    case NONE:
        return "CONST_INT";
    case STRING:
        return "STRING";
    case INTEGER:
        return "INTEGER";
    case KEYWORD:
        return "KEYWORD";
    case TYPE:
        return "TYPE";
    default:
    {
        char *s = malloc(sizeof(char));
        sprintf(s, "%d", value_type);
        return s;
    }
    }
}

// Parse value type to printf format for pretty print
// Token (<type>, <value type>, <value>)
char *value_type_to_format(token_value_type_t value_type)
{
    switch (value_type)
    {
    case NONE:
        return "Token (%s)";
    case INTEGER:
        return "Token (%s, %s, %s)";
    case STRING:
        return "Token (%s, %s, %s)";
    case KEYWORD:
        return "Token (%s, %s, %s)";
    case TYPE:
        return "Token (%s, %s, %s)";
    default:
        // default to whatever and add a note, it might work
        return "Token (%d, %d, %d, #invalid-value-type)";
    }
}

char *value_to_string(token_value_type_t type, token_value_t value)
{
    switch (type)
    {
    case STRING:
        return value.string;
    case INTEGER:
    {
        size_t len = snprintf(NULL, 0, "%d", value.integer);
        char *s = malloc(sizeof(char) * (len + 1));
        MALLOC_CHECK(s);
        sprintf(s, "%d", value.integer);
        return s;
    }
    case KEYWORD:
        return keyword_to_name(value.keyword);
    case TYPE:
        return type_to_name(value.type);
    default:
        return "#invalid-type-value";
    }
}

char *token_to_string(token_ptr token)
{
    char *format = value_type_to_format(token->value_type);

    char *value_s = value_to_string(token->value_type, token->value);

    size_t len = snprintf(NULL, 0,
                          format,
                          token_type_to_name(token->type),
                          value_type_to_name(token->value_type),
                          value_s);
    char *s = malloc((sizeof(char) * len) + 1);

    if (s == NULL)
    {
        fprintf(stderr, "malloc fail.\n");
        exit(FAIL_INTERNAL);
    }

    sprintf(s, format,
            token_type_to_name(token->type),
            value_type_to_name(token->value_type),
            value_s);

    if (token->value_type == INTEGER)
    {
        free(value_s);
    }
    return s;
}

void token_print(token_ptr token)
{
    char *s = token_to_string(token);
    // this actually does string things twice, could be ineffective,
    // but we should use this for debugging only, so it's fine
    printf("%s\n", s);
    free(s);
}

bool token_compare(token_ptr token1, token_ptr token2)
{
    if (token1->type != token2->type || token1->value_type != token2->value_type)
    {
        return false;
    }

    switch (token1->value_type)
    {
    case INTEGER:
        return token1->value.integer == token2->value.integer;
    case STRING:
        return strcmp(token1->value.string, token2->value.string) == 0;
    case FLOAT:
        return token1->value.float_value == token2->value.float_value;
    case TYPE:
        return token1->value.type == token2->value.type;
    case KEYWORD:
        return token1->value.keyword == token2->value.keyword;
    default:
        fprintf(stderr, "Don't know how to compare type %s.", value_type_to_name(token1->value_type));
        exit(FAIL_INTERNAL);
    }
}

bool is_one_of(token_ptr token, int count, ...)
{
    va_list va_list;
    va_start(va_list, count);

    for (int i = 0; i < count; i++)
    {
        if (va_arg(va_list, token_type_t) == token->type)
        {
            va_end(va_list);
            return true;
        }
    }

    va_end(va_list);
    return false;
}
