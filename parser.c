#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "symtable.h"
#include "stack.h"
#include "utils.h"

token_ptr peek_top(item_ptr *stack)
{
    symbol_ptr symbol = stack_top(*stack)->symbol;
    return symbol->token;
}

token_ptr peek_exact_type(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = peek_top(stack);
    return token->type == token_type ? token : NULL;
}

token_ptr get_next_token(item_ptr *stack)
{
    symbol_ptr symbol = stack_top(*stack)->symbol;
    token_ptr token = symbol->token;

    *stack = stack_pop(*stack);

    DEBUG_STACK_TOP(*stack, 2);

    free(symbol);
    return token;
}

token_ptr assert_next_token_get(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = get_next_token(stack);

    assert_token_type(token, token_type);

    return token;
}

void assert_token_type(token_ptr token, token_type_t type)
{
    DEBUG_ASSERT(type, token->type);

    if (token->type != type)
    {
        fprintf(stderr, "%s expected.\n", token_type_to_name(type));
        exit(1); // Return bool and bubble up instead?
    }
}

void assert_next_token(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = get_next_token(stack);

    assert_token_type(token, token_type);

    token_dispose(token);
}

void assert_next_keyword(item_ptr *stack, keyword_t keyword)
{
    token_ptr token = assert_next_token_get(stack, TOKEN_KEYWORD);

    if (token->value.keyword != keyword)
    {
        fprintf(stderr, "Expected keyword %s, got %s.\n", keyword_to_name(keyword), keyword_to_name(token->value.keyword));
        exit(1); // TODO: Correct code
    }

    token_dispose(token);
}

void assert_n_tokens(item_ptr *stack, int n, ...)
{
    va_list valist;
    va_start(valist, n);

    for (int i = 0; i < n; i++)
    {
        assert_next_token(stack, va_arg(valist, token_type_t));
    }

    va_end(valist);
}

int parse_expression(item_ptr *in_stack)
{
    DEBUG_RULE();

    token_ptr const_int = assert_next_token_get(in_stack, TOKEN_CONST_INT);

    int value = const_int->value.integer;
    token_dispose(const_int);
    return value;
}

// <statement> -> var_id = const_int;
// <statement> -> return <expression>;
// <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}
// <statement> -> while (<expression>) {<statement-list>}
// <statement> -> function id(<argument-list>) {<statement-list>}
void rule_statement(item_ptr *in_stack, tree_node_ptr tree)
{
    DEBUG_RULE();

    token_ptr next = get_next_token(in_stack);

    if (next->type == TOKEN_VAR_ID)
    {
        // <statement> -> var_id = <expression>;

        assert_next_token(in_stack, TOKEN_ASSIGN);

        int value = parse_expression(in_stack);

        // TODO: Create symboltable entry

        DEBUG_OUTF("%s <- %d", next->value.string, value);

        assert_next_token(in_stack, TOKEN_SEMICOLON);
    }
    else if (next->type == TOKEN_KEYWORD)
    {
        // if / function / while

        switch (next->value.keyword)
        {
        case KEYWORD_IF:
            // <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}

            // if (<expression>) {<statement-list>}

            assert_next_token(in_stack, TOKEN_L_PAREN);

            int value = parse_expression(in_stack);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_OUTF("if (%d)", value);

            rule_statement_list(in_stack, tree);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_OUT("end if");

            // else { <statement-list> }

            assert_next_keyword(in_stack, KEYWORD_ELSE);

            assert_next_token(in_stack, TOKEN_LC_BRACKET);

            DEBUG_OUT("else");

            rule_statement_list(in_stack, tree);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_OUT("end else");
            break;
        case KEYWORD_WHILE:
            // <statement> -> while (<expression>) {<statement-list>}
            // TODO: Implement
            assert_next_token(in_stack, TOKEN_L_PAREN);

            value = parse_expression(in_stack);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_OUTF("while (%d)", value);

            rule_statement_list(in_stack, tree);

            assert_next_token(in_stack, TOKEN_RC_BRACKET);

            DEBUG_OUT("end while");

            //fprintf(stderr, "Not implemented.\n");
          // exit(42);
            break;
        case KEYWORD_FUNCTION:
            // <statement> -> function id(<argument-list>) {<statement-list>}
            // TODO: Implement

           // assert_next_token(in_stack, TOKEN_ID);

            //assert_next_token(in_stack, TOKEN_L_PAREN);

            //int value = parse_expression(in_stack);
           // rule_argument_list(in_stack, tree);

           // assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

           // DEBUG_OUTF("function (%d)", value);

           // rule_statement_list(in_stack, tree);

           // assert_next_token(in_stack, TOKEN_RC_BRACKET);

           // DEBUG_OUT("end function");


            fprintf(stderr, "Not implemented.\n");
            exit(42);
            break;
        default:
            fprintf(stderr, "Invalid keyword in statement.\n");
            exit(1); // TODO: Correct code
        }
    }
    else
    {
        fprintf(stderr, "Invalid token.\n");
        exit(1); // TODO: Correct code
    }

    token_dispose(next);
}

// <statement-list> -> <statement><statement-list>
// <statement-list> -> eps
void rule_statement_list(item_ptr *in_stack, tree_node_ptr tree)
{
    DEBUG_RULE();

    // Decide based on first? There's always at least one statement

    token_ptr next = peek_top(in_stack);

    if ((next->type == TOKEN_KEYWORD) || next->type == TOKEN_VAR_ID || next->type == TOKEN_ID)
    {
        // <statement-list> -> <statement>;<statement-list>
        rule_statement(in_stack, tree);

        rule_statement_list(in_stack, tree);
    }
    else
    {
        return;
        // <statement-list> -> eps
    }
}
// <argument-list> -> type <expression> <argument-next>
// <argument-list> -> <expression> <argument-next>
// <argument-list> -> ε
//void rule_argument_list(item_ptr *in_stack, tree_node_ptr tree)
//{
 //   DEBUG_RULE();
//
 //   // Decide based on first? There's always at least one statement

 //   token_ptr next = peek_top(in_stack);
//
 //   if (next->type == TOKEN_KEYWORD || next->type == TOKEN_VAR_ID)
 //   {
 //       // <argument-list> -> type <expression> <argument-next>
 //       int value = parse_expression(in_stack);
 //        //to do
 //       //rule_(in_stack., tree);
//
  //      rule_statement_list(in_stack, tree);
  //  }
  //  //else if ()
  //  else
  //  {
  //      // <statement-list> -> eps
  // }
//}
// <prog> -> <?php <statement> ?>
void rule_prog(item_ptr *in_stack, tree_node_ptr tree)
{
    DEBUG_RULE();

    assert_n_tokens(in_stack, 3, TOKEN_LESS, TOKEN_NULLABLE, TOKEN_ID);
    // will work for any string needs to change
    assert_next_token(in_stack, TOKEN_ID);
    // all of this like that
    assert_next_token(in_stack, TOKEN_L_PAREN);
    assert_next_token(in_stack, TOKEN_ID);
    assert_next_token(in_stack, TOKEN_ASSIGN);
    assert_next_token(in_stack, TOKEN_CONST_INT);

    assert_next_token(in_stack, TOKEN_R_PAREN);
    assert_next_token(in_stack, TOKEN_SEMICOLON);

    rule_statement_list(in_stack, tree);

    rule_prog_end(in_stack, tree);
    
}

void rule_prog_end(item_ptr *in_stack, tree_node_ptr tree){
    token_ptr next = peek_top(in_stack);
    if (next->type == TOKEN_NULLABLE){
        assert_next_token(in_stack, TOKEN_NULLABLE);
        assert_next_token(in_stack, TOKEN_MORE);
    }
    

}
tree_node_ptr parse(array_ptr tokens)
{
    tree_node_ptr tree = tree_init();

    item_ptr in_stack = stack_init();

    // fill input stack from tokens
    element_ptr element = tokens->last;
    while (element != NULL)
    {
        symbol_ptr symbol = create_terminal(element->token);
        in_stack = stack_push(in_stack, symbol);
        element = element->prev;
    }

    rule_prog(&in_stack, tree);

    if (stack_size(in_stack) != 0)
    {
        fprintf(stderr, "SA failed, symbols left on the input stack.\n");
        exit(1); // TODO: Correct code
    }

    return tree;
}




/*
//// DIFFERNENT QUESTIONABLE ONES
void error_func(){
    exit(1);
}

token_ptr peek_top_(item_ptr *stack)
{
    symbol_ptr symbol = stack_top(*stack)->symbol;
    return symbol->token;
}

int assert_token_type_(token_ptr token, token_type_t type)
{
    DEBUG_ASSERT(type, token->type);

    if (token->type != type)
    {
        fprintf(stderr, "%s expected.\n", token_type_to_name(type));
        return 0;
    }
    return 1;
}

int assert_next_token_(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = get_next_token(stack);

    if (assert_token_type_(token, token_type)){
        return 1;
    }
    token_dispose(token);
    return 0;

}
token_ptr get_next_token_(item_ptr *stack)
{
    symbol_ptr symbol = stack_top(*stack)->symbol;
    token_ptr token = symbol->token;

    *stack = stack_pop(*stack);

    DEBUG_STACK_TOP(*stack, 2);

    free(symbol);
    return token;
}
int prog_(item_ptr *in_stack, tree_node_ptr tree){
    if (!assert_next_token_(in_stack, TOKEN_LESS)) error_func();
    if (!assert_next_token_(in_stack, TOKEN_LESS)) error_func();
    if (!assert_next_token_(in_stack, TOKEN_NULLABLE)) error_func();
    // php add somehow
    if (!assert_next_token_(in_stack, TOKEN_L_PAREN)) error_func();
    // id for stricttypes=1
    if (!assert_next_token_(in_stack, TOKEN_R_PAREN)) error_func();
    if (!assert_next_token_(in_stack, TOKEN_SEMICOLON)) error_func();

    if (!statement_list_(in_stack, tree)) error_func();

    if (!prog_end_(in_stack, tree)) error_func();
    
    // php add somehow 
    return 1;

}

int prog_end_(item_ptr *in_stack, tree_node_ptr tree){
    token_ptr next = peek_top(in_stack);
    if (next->type == TOKEN_NULLABLE){
        if (!assert_next_token_(in_stack, TOKEN_NULLABLE)) error_func();
        if (!assert_next_token_(in_stack, TOKEN_MORE)) error_func();
    }
    return 1;
}
int statement_list_(item_ptr *in_stack, tree_node_ptr tree){
    token_ptr next = peek_top(in_stack);
    // token id == meno funkcie ?
    if (((next->type == KEYWORD) || next->type == TOKEN_VAR_ID || next->type == TOKEN_ID)){
        statement_(in_stack, tree);
        statement_list_(in_stack, tree);
    }
    return 1;
}
int statement_(item_ptr *in_stack, tree_node_ptr tree){
    token_ptr next = get_next_token_(in_stack);
    if (next->value.keyword == KEYWORD_RETURN) expression_(in_stack, tree);
    if (next->value.keyword == KEYWORD_WHILE){
        if (!assert_next_token_(in_stack, TOKEN_L_PAREN)) error_func();
        expression_(in_stack, tree);
        // need to call in if depending on what it returns
        if (!assert_next_token_(in_stack, TOKEN_R_PAREN)) error_func();
    }
    if (next->value.keyword == KEYWORD_IF);
    if (next->value.keyword == KEYWORD_FUNCTION);
    if (next->type == TOKEN_VAR_ID);
    if (next->type == TOKEN_ID);

}
int argument_list_(item_ptr *in_stack, tree_node_ptr tree);
int argument_next_(item_ptr *in_stack, tree_node_ptr tree);
int argument_list_typ_(item_ptr *in_stack, tree_node_ptr tree);
int argument_next_typ(item_ptr *in_stack, tree_node_ptr tree);
int expression_(item_ptr *in_stack, tree_node_ptr tree);
int expression_tail(item_ptr *in_stack, tree_node_ptr tree);
*/