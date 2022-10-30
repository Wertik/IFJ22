#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "token.h"

int main(int argc, char const *argv[])
{
    // Initialize a token array.
    array_ptr tokens = array_create();
    
    // Tokenize stdin
    tokenize(tokens);

    // Print the result & exit.
    array_print(tokens);
    array_dispose(tokens);

    return 0;
}
