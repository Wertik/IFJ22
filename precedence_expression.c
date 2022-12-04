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
//   +-. ;   */ ;     (  ;     ) ;    i ;     <?(;) ; === !== ; ><<=>= 
    {'>',    '<',     '<',     '>',     '<',     '>',     '>',     '>'}, //+-.
    {'>',    '>',     '<',     '>',     '<',     '>',     '>',     '>'}, //*/
    {'<',    '<',     'n',     '=',     '<',     'x',     '>',     '<'}, // (
    {'>',    '>',     'n',     'n',     'n',     'x',     '>',     '>'}, // ) 
    {'>',    '>',     'n',     '>',     '<',     '>',     '>',     '>'}, // i 
    {'<',    '<',     '<',     'n',     '<',     'n',     '<',     '<'}, // <? (;)
    {'<',    '<',     '<',     '>',     '<',     '>',     '>',     '<'}, //===,!===
    {'<',    '<',     '<',     '>',     '<',     '>',     '>',     '>'}, // <><=>=
};

token_ptr get_first_non_E(stack_ptr stack){
    DEBUG("GET FIRST NON E\n");
    token_ptr next = peek_top(stack);
    //token_value_t value;
    if (next->type == TOKEN_CONST_EXP){
        DEBUG("CONST_EXP\n");
        token_ptr skipped = get_next_token(stack);
        next = peek_top(stack);

       // token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
      // symbol_ptr symbol = create_terminal(E);

        symbol_ptr symbol = create_terminal(skipped);
        stack_push(stack, symbol);
      //  stack_push(stack, symbol);
    }
    if (next == NONE){printf("fuck");}
    return next;
}
int get_pos_in_t(token_ptr TOKEN)
{
    DEBUG("GET_POS_T\n");
    switch (TOKEN->type)
    {
    case TOKEN_SEMICOLON :
        DEBUG("5\n");
        return 5;
        break;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_DOT:
        DEBUG("0\n");
        return 0;
        break;
    case TOKEN_DIVIDE:
    case TOKEN_MULTIPLE:
        DEBUG("1\n");
        return 1;
        break;
    case TOKEN_L_PAREN:
        DEBUG("2\n");
        return 2;
        break;
    case TOKEN_R_PAREN:
        DEBUG("3\n");
        return 3;
        break;
    case TOKEN_LESS:
    case TOKEN_MORE:
    case TOKEN_LESS_EQUAL:
    case TOKEN_MORE_EQUAL:
        DEBUG("7\n");
        return 7;
        break;
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
        DEBUG("6\n");
        return 6;
        break;
    case TOKEN_STRING_LIT:
    case TOKEN_CONST_INT:
    case TOKEN_CONST_DOUBLE:
        DEBUG("4\n");
        return 4;
        break;
    case TOKEN_CONST_EXP:
        DEBUG(" FAILED\n");
        fprintf(stderr, "EXP SHOULD NOT END UP HERE");
        exit(FAIL_LEXICAL);
        break;
    default:
        DEBUG(" RAN TO END\n");
        fprintf(stderr, "NON VALID TOKEN");
        exit(FAIL_LEXICAL);
        break;
    }
    DEBUG(" RAN TO END\n");
    fprintf(stderr, "NON VALID TOKEN");
    exit(FAIL_LEXICAL);
}
void perform_reduction(stack_ptr push_down_stack, sym_table_ptr tree){
    DEBUG("PERFORMING REDUCTION\n");
    token_ptr next = get_next_token(push_down_stack);
    DEBUG("hhh\n");
    token_value_t value;
    if (next->type == TOKEN_CONST_INT){
        DEBUG("HERE1\n");
        //stack_pop(push_down_stack);
        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        DEBUG("gN\n");
        DEBUG_PSEUDO("$");
        DEBUG("g\n");
        symbol_ptr symbol = create_terminal(E);
        DEBUG("1\n");
        stack_push(push_down_stack, symbol);
        DEBUG("2\n");
        return;
    }
    if (next->type == TOKEN_VAR_ID){
        DEBUG("HERE2\n");
        //stack_pop(push_down_stack);
        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        DEBUG_PSEUDO("$");
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);
        return;
    }
    if (next->type == TOKEN_CONST_EXP)
    {
        DEBUG("HERE3\n");
        token_ptr second_next = get_next_token(push_down_stack);
        if (second_next->type != TOKEN_SEMICOLON){
            stack_pop(push_down_stack);
            token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
            DEBUG_PSEUDO("$");
            symbol_ptr symbol = create_terminal(E);
            stack_push(push_down_stack, symbol);
            return;
        }
    }
    fprintf(stderr, "NOT POSSIBLE TO REDUCE");
    exit(FAIL_LEXICAL);
}

void perform_addition(stack_ptr push_down_stack, sym_table_ptr tree , stack_ptr in_stack){
    DEBUG("PERFORMING ADITION\n");
    token_ptr next = get_next_token(in_stack);
    symbol_ptr symbol = create_terminal(next);
    DEBUG("Pj\n");
    stack_push(push_down_stack, symbol);
    DEBUG("jN\n");
    token_ptr bruh = peek_top(push_down_stack);
    if (bruh->type == TOKEN_CONST_INT){
        DEBUG("IT WORKED \n");
    }
    //assert_next_token(push_down_stack,TOKEN_CONST_INT);
}

int expression_prec(stack_ptr in_stack, sym_table_ptr tree, stack_ptr push_down_stack){
    DEBUG_RULE();
    DEBUG("ENTERING EXPRESSION PARSER BOTTOM UP \n");

    // dollar = create_terminal(dollar);
    DEBUG_PSEUDO("$"); // not sure what this is for
    // changed stuff here
    token_ptr next_sym_stack = peek_top(in_stack);
    token_ptr next_sym_push = get_first_non_E(push_down_stack);
    int next_in_push = get_pos_in_t(next_sym_push);
    DEBUG("EHY\n");
    int next_in_stack = get_pos_in_t(next_sym_stack);
    DEBUG("WHY\n");
    if (precedence_table[next_in_push][next_in_stack] == '>')
    {
        DEBUG("WHY AM I NOT HERE \n");
        perform_reduction(push_down_stack, tree);
    }
    else if (precedence_table[next_in_push][next_in_stack] == '<')
    {
        perform_addition(push_down_stack, tree, in_stack);
        
        
        token_ptr bruh = peek_top(push_down_stack);
        if (bruh->type == TOKEN_CONST_INT){
            DEBUG("IT WORKED \n");
        }
    }
    else if (precedence_table[next_in_push][next_in_stack] == '=')
    {
        // not finnished
    }
    else if (precedence_table[next_in_push][next_in_stack] == 'n')
    {
        exit(FAIL_LEXICAL);
        fprintf(stderr, "ERROR EXPRESSION NOT IN CORRECT ORDER");
    }
    //not finnished
    bool finnish = ((get_first_non_E(push_down_stack)->type == TOKEN_SEMICOLON) && (peek_top(in_stack)->type == TOKEN_SEMICOLON));  
    if (finnish == true){
        DEBUG(" I FINNISHED PARSING EXP \n");
        return 0;
    }
    else {
        expression_prec(in_stack, tree, push_down_stack);
    }
    DEBUG(" AM I HERE ? \n");
    return 0;
}