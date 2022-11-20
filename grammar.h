#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <stdbool.h>
#include "token.h"

typedef enum
{
    NONTERMINAL_PROG,
    NONTERMINAL_STATEMENT,
    NONTERMINAL_STATEMENT_LIST,
    NONTERMINAL_EXPRESSION,
    // TODO: Add more
} nonterminal_t;

typedef struct symbol_t
{
    bool terminal;
    // only if terminal = false
    nonterminal_t nonterminal_type;
    // only if terminal = true
    token_ptr token;
} * symbol_ptr;

symbol_ptr create_nonterminal(nonterminal_t type);
symbol_ptr create_terminal(token_ptr token);

char *keyword_to_name(keyword_t keyword); 

char *symbol_to_string(symbol_ptr symbol);
void symbol_print(symbol_ptr symbol);
char *nonterminal_to_string(nonterminal_t type);

void symbol_dispose(symbol_ptr symbol);

#endif
