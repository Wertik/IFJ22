#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stack.h"
#include "parser.h"
#include "utils.h"

char precedence_table[8][8] ={
//   +-. ;   */ ;     (  ;     ) ;    i ;     <? ; === !== ; ><<=>= 
    {'>',    '<',     '<',     '>',     '<',     '>',     '>',     '>'}, //+-.
    {'>',    '>',     '<',     '>',     '<',     '>',     '>',     '>'}, //*/
    {'<',    '<',     'n',     '=',     '<',     'nev',     '>',     '<'}, // (
    {'>',    '>',     'n',     'n',     'n',     'nev',     '>',     '>'}, // ) 
    {'>',    '>',     'n',     '>',     '<',     '>',     '>',     '>'}, // i 
    {'<',    '<',     '<',     'n',     '<',     'n',     '<',     '<'}, // <?
    {'<',    '<',     '<',     '>',     '<',     '>',     '>',     '<'}, //===,!===
    {'<',    '<',     '<',     '>',     '<',     '>',     '>',     '>'}, // <><=>=
};

void perform_reduction(item_ptr *push_down_stack, table_node_ptr *tree){
    token_ptr next = get_next_token(push_down_stack);
    token_value_t value;
    if (next->type == TOKEN_VAR_ID){
        //stack_pop(push_down_stack);
        // am not sure about different types int string float
        symbol_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        DEBUG_OUT("$");
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);
    }
    if (next->type == TOKEN_CONST_EXP){
        token_ptr second_next = get_next_token(push_down_stack);
        if (second_next->type != TOKEN_OPENING_TAG){
            stack_pop(push_down_stack);
            symbol_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
            DEBUG_OUT("$");
            symbol_ptr symbol = create_terminal(E);
            stack_push(push_down_stack, symbol);
        }
    }
    exit(1);
    fprintf(stderr,"NOT POSSIBLE TO REDUCE");

}

void perform_addition(item_ptr *push_down_stack, table_node_ptr *tree , item_ptr *in_stack){
    token_ptr next = get_next_token(in_stack);
    symbol_ptr symbol = create_terminal(next);
    stack_push(push_down_stack, symbol);

}

int expression(item_ptr *in_stack, table_node_ptr *tree){
    
    item_ptr push_down_stack = stack_init();

    // represents dollar
    token_value_t value;
    token_ptr dollar = token_create(TOKEN_OPENING_TAG, NONE, value);
    symbol_ptr symbol = create_terminal(dollar);

    //dollar = create_terminal(dollar);
    DEBUG_OUT("$");
    push_down_stack = stack_push(in_stack, symbol);
    token_ptr next_in_push = peek_top(push_down_stack);
    switch (next_in_push->type)
    {
    case TOKEN_OPENING_TAG :
        /* code */
        break;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_DOT:
        /* code */
        break;
    case TOKEN_DIVIDE:
    case TOKEN_MULTIPLE:
        /* code */
        break;
    case TOKEN_L_PAREN:
        /* code */
        break;
    case TOKEN_R_PAREN:
        /* code */
        break;
    case TOKEN_LESS:
    case TOKEN_MORE:
    case TOKEN_LESS_EQUAL:
    case TOKEN_MORE_EQUAL:
        /* code */
        break;
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
        /* code */
        break;
    case TOKEN_STRING_LIT:
        /* code */
        break;
    case TOKEN_CONST_INT:
        /* code */
        break;
    case TOKEN_CONST_DOUBLE:
        /* code */
        break;
    case TOKEN_CONST_EXP:
        /* code */
        break;
    default:
        break;
    }
}