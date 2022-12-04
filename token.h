/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xbalek01 Miroslav Bálek
 * @author xdobes22 Kristián Dobeš
 */

#ifndef _TOKEN_H
#define _TOKEN_H

#include <stdbool.h>

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
    NONE, // token holds no value
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
    TOKEN_L_PAREN,      // (
    TOKEN_R_PAREN,      // )
    TOKEN_CONST_INT,    // Integer constant
    TOKEN_CONST_DOUBLE, // Float constant
    TOKEN_STRING_LIT,   // String literal
    TOKEN_RC_BRACKET,   // }
    TOKEN_LC_BRACKET,   // {
    TOKEN_NULLABLE,     // ?
    TOKEN_COLON,        // :
    TOKEN_DOT,          // .
    TOKEN_VAR_ID,       // variable name starting with $
    TOKEN_COMMA,        // ,
    TOKEN_SEMICOLON,    // ;
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULTIPLE,     // *
    TOKEN_DIVIDE,       // /
    TOKEN_MORE,         // >
    TOKEN_MORE_EQUAL,   // >=
    TOKEN_LESS,         // <
    TOKEN_LESS_EQUAL,   // <=
    TOKEN_EQUAL,        // ===
    TOKEN_ASSIGN,       // =
    TOKEN_NOT_EQUAL,    // !==
    TOKEN_CONST_EXP,    // no idea
    TOKEN_ID,           // function id
    TOKEN_KEYWORD,      // one of the predefined keywords
    TOKEN_TYPE,         // one of the predefined types
    TOKEN_CLOSING_TAG,  // ?>
    TOKEN_OPENING_TAG,  // <?php
    TOKEN_DECLARE,      // declare(strict_types=1);
} token_type_t;

typedef struct token_t
{
    token_type_t type;
    token_value_type_t value_type;
    token_value_t value;
} * token_ptr;

// Create a token.
token_ptr token_create(token_type_t type, token_value_type_t value_type, token_value_t value);

// Create an empty token.
// Empty value, value type NONE
token_ptr token_create_empty(token_type_t type);

// Create a string value token.
// Copies the input string.
// String value, value type STRING
token_ptr token_create_string(token_type_t type, const char *str);

// Create a keyword token.
// TOKEN_KEYWORD, keyword value, type KEYWORD.
token_ptr token_create_keyword(keyword_t keyword);
// Create a type token.
// TOKEN_TYPE, type value, type TYPE.
token_ptr token_create_type(type_t type);

// Return true if a token is one of the types.
bool is_one_of(token_ptr token, int count, ...);

// Convert a token to string.
// Returns an allocated string.
char *token_to_string(token_ptr token);

// Convert a token type enum to string.
const char *token_type_to_name(token_type_t type);

// Print a token with fashion.
void token_print(token_ptr token);

// Compare two tokens.
// Compare type, value type and value.
// Return true if they're the same.
bool token_compare(token_ptr token1, token_ptr token2);

// Dispose of a token with fashion.
void token_dispose(token_ptr token);

// Convert type enum to name.
const char *type_to_name(type_t type);
// Convert type enum to a formal valid ifjcode22 type name.
const char *type_to_formal(type_t type);

// Convert a token value to string. Helper for token_to_string
char *value_to_string(token_value_type_t type, token_value_t value);

// Convert a value type to name.
const char *value_type_to_name(token_value_type_t value_type);
// Return a format for value type. Helper for token_to_string
const char *value_type_to_format(token_value_type_t value_type);
// Convert keyword enum to name.
const char *keyword_to_name(keyword_t keyword);

#endif