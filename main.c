/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 */

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "parser.h"
#include "token.h"
#include "symtable.h"
#include "utils.h"

int main(int argc, char const *argv[])
{
    // Tokenize stdin
    stack_ptr stack = tokenize();

    // Print tokens
    DEBUG_STACK(stack);

    // Parse tokens
    parse(stack);

    stack_dispose(stack);
    return 0;
}
