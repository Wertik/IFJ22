#ifndef _TOKEN_H
#define _TOKEN_H

#include <stdbool.h>

/* Token definitions */

typedef enum
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_VOID,
} type_t;

typedef enum
{
    KEYWORD_FUNCTION,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_RETURN,
} keyword_t;

typedef union
{
    double float_value;
    int integer;
    char *string;
    keyword_t keyword;
    type_t type;
} token_value_t;

typedef enum
{
    NONE,
    INTEGER,
    STRING,
    FLOAT,
    /* Token is a TOKEN_KEYWORD, this indicates which of the predefined keywords. */
    KEYWORD,
    /* Token is a TOKEN_TYPE, this indicates which of the predefined types. */
    TYPE,
} token_value_type_t;

typedef enum
{
    TOKEN_L_PAREN,    // Left parenthesses
    TOKEN_R_PAREN,    // Right parenthesses
    TOKEN_CONST_INT,  // Integer constant
    TOKEN_STRING_LIT, // String literal
    TOKEN_RC_BRACKET,
    TOKEN_LC_BRACKET,
    TOKEN_NULLABLE,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_VAR_ID,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLE,
    TOKEN_DIVIDE,
    TOKEN_MORE,
    TOKEN_MORE_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_CONST_DOUBLE,
    TOKEN_EQUAL,
    TOKEN_ASSIGN,
    TOKEN_NOT_EQUAL,
    TOKEN_CONST_EXP,
    TOKEN_ID,
    TOKEN_KEYWORD,
    /* Variable type / function return type. */
    TOKEN_TYPE,
    TOKEN_CLOSING_TAG,
    TOKEN_OPENING_TAG,
    TOKEN_DECLARE,
} token_type_t;

typedef struct token_t
{
    token_type_t type;
    token_value_type_t value_type;
    token_value_t value;
} * token_ptr;

/* Token functions */

token_ptr token_create(token_type_t type, token_value_type_t value_type, token_value_t value);

token_ptr token_create_empty(token_type_t type);

token_ptr token_create_string(token_type_t type, char *str);

token_ptr token_create_keyword(keyword_t keyword);
token_ptr token_create_type(type_t type);

bool is_one_of(token_ptr token, int count, ...);

char *token_to_string(token_ptr token);
char *token_type_to_name(token_type_t type);

void token_print(token_ptr token);

bool token_compare(token_ptr token1, token_ptr token2);

void token_dispose(token_ptr token);

char *type_to_name(type_t type);
char *keyword_to_name(keyword_t keyword); 

#endif