#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include "array.h"

/* States */
#define SCANNER_START 1
#define SCANNER_STRING 2
#define SCANNER_VAR_ID_START 3
#define SCANNER_STRING_ESCAPE 4
#define SCANNER_HEX_START 5
#define SCANNER_HEX_FIRST 6
#define SCANNER_OCTA_1 7
#define SCANNER_OCTA_2 8
#define SCANNER_ASIGN 9
#define SCANNER_EQ_START 10
#define SCANNER_EXCL_MARK 11
#define SCANNER_NOT_EQ_START 12
#define SCANNER_NUM_INT 13
#define SCANNER_NUM_POINT_START 14
#define SCANNER_NUM_DOUBLE 15
#define SCANNER_NUM_EXP_START 16
#define SCANNER_NUM_EXP_SIGN 17
#define SCANNER_NUM_EXP 18
#define SCANNER_LESS_THAN 19
#define SCANNER_MORE_THAN 20
#define SCANNER_DIVIDE 21
#define SCANNER_LINE_COMM 22
#define SCANNER_BLOCK_COMM 23
#define SCANNER_BLOCK_END 24
#define SCANNER_ID 25
#define SCANNER_VAR_ID 26
#define SCANNER_PROG_START 27
#define SCANNER_NULLABLE 28

/* Read from stdin, parse into tokens. */
void tokenize(array_ptr tokens);

void append_token(array_ptr tokens, token_type_t token_type);

void append_keyword(array_ptr tokens, keyword_t keyword);
void append_type(array_ptr tokens, type_t type);

#endif