/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 */

#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <stdbool.h>
#include "token.h"

typedef enum
{
    NONTERMINAL_PROG,
    NONTERMINAL_STATEMENT,
    NONTERMINAL_STATEMENT_LIST,
    NONTERMINAL_EXPRESSION,
} nonterminal_t;

typedef struct symbol_t
{
    bool terminal;
    // only if terminal = false
    nonterminal_t nonterminal_type;
    // only if terminal = true
    token_ptr token;
} * symbol_ptr;

// Create a non terminal symbol.
symbol_ptr create_nonterminal(nonterminal_t type);

// Create a terminal symbol.
symbol_ptr create_terminal(token_ptr token);

// Convert a symbol to string.
// Returns an alloc'd string.
char *symbol_to_string(symbol_ptr symbol);

// Print the symbol with fashion.
void symbol_print(symbol_ptr symbol);

// Converts a nonterminal enum to string name.
const char *nonterminal_to_string(nonterminal_t type);

// Dispose of the symbol.
// Disposes of associated token if terminal = true.
void symbol_dispose(symbol_ptr symbol);

#endif
