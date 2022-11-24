#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "parser.h"
#include "token.h"
#include "symtable.h"
#include "utils.h"

int main(int argc, char const *argv[])
{
    // Initialize a token array.
    array_ptr tokens = array_create();

    // Tokenize stdin
    tokenize(tokens);

    // Print tokens
    DEBUG_ARRAY(tokens);

    // Parse tokens
    parse(tokens);

    array_dispose(tokens);

    printf("Finished without errors!\n");
    return 0;
}
