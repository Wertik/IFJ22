/*
 * Project: IFJ22 language compiler
 *
 * @author xsynak03 Maroš Synák
 * @author xotrad00 Martin Otradovec
 */

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

token_ptr get_first_non_E(item_ptr *stack){
    token_ptr next = peek_top(stack);
    token_value_t value;
    if (next->type == TOKEN_CONST_EXP){
        token_ptr skiped = get_next_token(stack);
        next = peek_top(stack);

        symbol_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        stack_push(stack, E);
    }
    return next;
}
int get_pos_in_t(token_ptr TOKEN){
    switch (TOKEN->type)
    {
    case TOKEN_OPENING_TAG :
        return 5;
        break;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_DOT:
        return 0;
        break;
    case TOKEN_DIVIDE:
    case TOKEN_MULTIPLE:
        return 1;
        break;
    case TOKEN_L_PAREN:
        return 2;
        break;
    case TOKEN_R_PAREN:
        return 3;
        break;
    case TOKEN_LESS:
    case TOKEN_MORE:
    case TOKEN_LESS_EQUAL:
    case TOKEN_MORE_EQUAL:
        return 7;
        break;
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
        return 6;
        break;
    case TOKEN_STRING_LIT:
    case TOKEN_CONST_INT:
    case TOKEN_CONST_DOUBLE:
        return 4;
        break;
    case TOKEN_CONST_EXP:
        fprintf(stderr, "EXP SHOULD NOT END UP HERE");
        exit(FAIL_LEXICAL);
        break;
    default:
        break;
    }
    fprintf(stderr, "NON VALID TOKEN");
    exit(FAIL_LEXICAL);
}
void perform_reduction(item_ptr *push_down_stack, table_node_ptr *tree){
    token_ptr next = get_next_token(push_down_stack);
    token_value_t value;
    if (next->type == TOKEN_VAR_ID){
        //stack_pop(push_down_stack);
        // am not sure about different types int string float
        symbol_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        DEBUG_PSEUDO("$");
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);
    }
    if (next->type == TOKEN_CONST_EXP){
        token_ptr second_next = get_next_token(push_down_stack);
        if (second_next->type != TOKEN_OPENING_TAG){
            stack_pop(push_down_stack);
            symbol_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
            DEBUG_PSEUDO("$");
            symbol_ptr symbol = create_terminal(E);
            stack_push(push_down_stack, symbol);
        }
    }
    exit(FAIL_LEXICAL);
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
    DEBUG_PSEUDO("$");
    stack_push(push_down_stack, symbol);
    token_ptr next_sym_stack = peek_top(in_stack);
    token_ptr next_sym_push = get_first_non_E(push_down_stack);
    int next_in_push = get_pos_in_t(next_sym_push);
    int next_in_stack = get_pos_in_t(next_sym_stack);
    if (precedence_table[next_in_push][next_in_stack] == '>'){
        
        perform_reduction(push_down_stack, tree);
    }
    else if (precedence_table[next_in_push][next_in_stack] == '<'){
       // not finnished
    }
    else if (precedence_table[next_in_push][next_in_stack] == '='){
       // not finnished
    }
    else if (precedence_table[next_in_push][next_in_stack] == 'n'){
        exit(FAIL_LEXICAL);
        fprintf(stderr,"ERROR EXPRESSION NOT IN CORRECT ORDER");
    }
}