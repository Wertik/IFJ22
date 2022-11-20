#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "grammar.h"

symbol_ptr create_nonterminal(nonterminal_t type)
{
    symbol_ptr symbol = malloc(sizeof(struct symbol_t));

    if (symbol == NULL)
    {
        fprintf(stderr, "create_nonterminal: malloc fail\n");
        exit(99);
    }

    symbol->terminal = false;
    symbol->nonterminal_type = type;

    return symbol;
}

symbol_ptr create_terminal(token_ptr token)
{
    symbol_ptr symbol = malloc(sizeof(struct symbol_t));

    if (symbol == NULL)
    {
        fprintf(stderr, "create_terminal: malloc fail\n");
        exit(99);
    }

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
    // TODO: Add more
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

        if (s == NULL)
        {
            fprintf(stderr, "symbol_to_string: malloc fail.\n");
            exit(99);
        }

        sprintf(s, "Symbol (terminal, %s)", token_s);
        free(token_s);
        return s;
    }
    else
    {
        char *nonterminal_s = nonterminal_to_string(symbol->nonterminal_type);
        size_t len = snprintf(NULL, 0, "symbol(nonterminal, %s)", nonterminal_s);

        char *s = malloc((sizeof(char) * len) + 1);

        if (s == NULL)
        {
            fprintf(stderr, "symbol_to_string: malloc fail.\n");
            exit(99);
        }

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

char *keyword_to_name(keyword_t keyword)
{
    switch (keyword)
    {
    case KEYWORD_IF:
        return "KEYWORD_IF";
    case KEYWORD_ELSE:
        return "KEYWORD_ELSE";
    case KEYWORD_WHILE:
        return "KEYWORD_WHILE";
    case KEYWORD_FUNCTION:
        return "KEYWORD_FUNCTION";
    case KEYWORD_RETURN:
        return "KEYWORD_RETURN";
    default:
        return "#unknown-keyword";
    }
}

void symbol_dispose(symbol_ptr symbol)
{
    free(symbol);
}
