#ifndef TOKEN_H
#define TOKEN_H

/* Token definitions */

typedef union
{
    int integer;
    char *string;
    // TODO: Add other type values
} token_value_t;

typedef enum
{
    NONE,
    INTEGER,
    STRING,
    // TODO: Add decimals and other types
} token_value_type_t;

typedef enum
{
    TOKEN_L_PAREN,    // Left parenthesses
    TOKEN_R_PAREN,    // Right parenthesses
    TOKEN_CONST_INT,  // Integer constant
    TOKEN_STRING_LIT, // String literal
    TOKEN_RC_BRACKET,
    TOKEN_LC_BRACKET,
    TOKEN_NULLABLE_START,
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
    TOKEN_ASIGN,
    TOKEN_NOT_EQUAL,
    TOKEN_CONST_EXP,
    TOKEN_ID

    // TODO: Add token types
} token_type_t;

typedef struct token_t
{
    token_type_t type;
    token_value_type_t value_type;
    token_value_t value;
} * token_ptr;

/* Token functions */

token_ptr token_create(token_type_t type, token_value_type_t value_type, token_value_t value);

char *token_to_string(token_ptr token);

void token_print(token_ptr token);

void token_dispose(token_ptr token);

#endif