#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "parser.h"
#include "token.h"
#include "symtable.h"

int main(int argc, char const *argv[])
{
    // Initialize a token array.
    array_ptr tokens = array_create();
    
    // Tokenize stdin
    tokenize(tokens);

    // Print tokens
    array_print(tokens);

    // Parse tokens
    parse(tokens);

    array_dispose(tokens);

    return 0;
}
