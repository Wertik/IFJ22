#include <stdlib.h>
#include <stdio.h>
#include "token.h"
#include "array.h"

token_ptr token_create(token_type_t type, token_value_type_t value_type, token_value_t value)
{
    token_ptr token = (token_ptr)malloc(sizeof(struct token_t));

    if (token == NULL)
    {
        // TODO: Handle better?
        return NULL;
    }

    // TODO: What if we enter a value that's incompatible with given value type?
    // We could map each type into an exact value type and force it.

    token->type = type;
    token->value_type = value_type;
    token->value = value;

    return token;
}

void token_dispose(token_ptr token)
{
    // free string values
    if (token->value_type == STRING) {
        free(token->value.string);
    }

    free(token);
}

// Parse type name to string for pretty print
char *type_to_name(token_type_t type)
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
    case TOKEN_NULLABLE_START:
        return "TOKEN_NULLABLE_START";
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
    case TOKEN_ASIGN:
        return "TOKEN_ASIGN";
    case TOKEN_NOT_EQUAL:
        return "TOKEN_NOT_EQUAL";
    case TOKEN_CONST_EXP:
        return "TOKEN_CONST_EXP";
    case TOKEN_ID:
        return "TOKEN_ID";
    default:
    {
        char *s = malloc(sizeof(char));
        sprintf(s, "%d", type);
        return s;
    }
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
        return "Token (%s, %s, %d)";
    case STRING:
        return "Token (%s, %s, %s)";
    default:
        // default to whatever and add a note, it might work
        return "Token (%d, %d, %d, #ivalid-value-type)";
    }
}

char *token_to_string(token_ptr token)
{
    size_t len = snprintf(NULL, 0, value_type_to_format(token->value_type), type_to_name(token->type), value_type_to_name(token->value_type), token->value);
    char *s = malloc((sizeof(char) * len) + 1);

    if (s == NULL)
    {
        // TODO: Handle better?
        fprintf(stderr, "malloc fail.\n");
        exit(99);
    }

    sprintf(s, value_type_to_format(token->value_type), type_to_name(token->type), value_type_to_name(token->value_type), token->value);
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
