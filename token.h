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