/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "symbol.h"
#include "utils.h"

symbol_ptr create_nonterminal(nonterminal_t type)
{
    symbol_ptr symbol = malloc(sizeof(struct symbol_t));

    MALLOC_CHECK(symbol);

    symbol->terminal = false;
    symbol->nonterminal_type = type;

    return symbol;
}

symbol_ptr create_terminal(token_ptr token)
{
    symbol_ptr symbol = malloc(sizeof(struct symbol_t));

    MALLOC_CHECK(symbol);

    symbol->terminal = true;
    symbol->token = token;

    return symbol;
}

char *nonterminal_to_string(nonterminal_t type)
{
    switch (type)
    {
    case NONTERMINAL_PROG:
        return "PROG";
    case NONTERMINAL_STATEMENT:
        return "STATEMENT";
    case NONTERMINAL_STATEMENT_LIST:
        return "STATEMENT_LIST";
    case NONTERMINAL_EXPRESSION:
        return "EXPRESSION";
    default:
        return "#unknown-nonterminal";
    }
}

char *symbol_to_string(symbol_ptr symbol)
{
    if (symbol->terminal == true)
    {
        char *token_s = token_to_string(symbol->token);
        size_t len = snprintf(NULL, 0, "Symbol (terminal, %s)", token_s);

        char *s = malloc((sizeof(char) * len) + 1);

        MALLOC_CHECK(s);

        sprintf(s, "Symbol (terminal, %s)", token_s);
        free(token_s);
        return s;
    }
    else
    {
        char *nonterminal_s = nonterminal_to_string(symbol->nonterminal_type);
        size_t len = snprintf(NULL, 0, "symbol(nonterminal, %s)", nonterminal_s);

        char *s = malloc((sizeof(char) * len) + 1);

        MALLOC_CHECK(s);

        sprintf(s, "Symbol (nonterminal, %s)", nonterminal_s);
        return s;
    }
}

void symbol_print(symbol_ptr symbol)
{
    char *s = symbol_to_string(symbol);
    printf("%s\n", s);
    free(s);
}

void symbol_dispose(symbol_ptr symbol)
{
    if (symbol->terminal == true) {
        token_dispose(symbol->token);
    }
    free(symbol);
}
