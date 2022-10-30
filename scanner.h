#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include "array.h"

/* States */
#define SCANNER_START 1
#define SCANNER_STRING 2
// TODO: States from FSM...

/* Read from stdin, parse into tokens. */
void tokenize(array_ptr tokens);

#endif