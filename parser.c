#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "symtable.h"
#include "stack.h"
#include "utils.h"

// hack for now for knowing if function requires return
int expect_return = 0;
int expected_type = -10;
int different_parse = 0;

token_ptr peek_top(item_ptr *stack)
{
    item_ptr top = stack_top(*stack);

    if (top == NULL)
    {
        return NULL;
    }
    symbol_ptr symbol = stack_top(*stack)->symbol;
    return symbol->token;
}

token_ptr peek_exact_type(item_ptr *stack, token_type_t token_type)
{
    token_ptr token = peek_top(stack);
    if (token == NULL)
    {
        return NULL;
    }
    return token->type == token_type ? token : NULL;
}

token_ptr get_next_token(item_ptr *stack)
{
    symbol_ptr symbol = stack_top(*stack)->symbol;
    token_ptr token = symbol->token;

    *stack = stack_pop(*stack);

    DEBUG_TOKEN(token);
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

// treba asi zmenit
int parse_expression(item_ptr *in_stack)
{
    DEBUG_RULE();
    // for function if its type void and so checks if it returns anything
    if (different_parse != 0){
        token_ptr type = peek_top(in_stack);
        DEBUG_OUT("shouldnt be here? \n");
        if (expected_type == -1){
            DEBUG_OUT("I AM HERE? \n");
            if (type->type != TOKEN_SEMICOLON){
                fprintf(stderr,"RETURN SHOULD RETURN NOTHING\n");
                exit (1);
            }
            expected_type = 0;
            return 0;
        }
        else{
            DEBUG_OUTF("shouldnt be here? %d \n", (type->value.type));
            DEBUG_OUTF("shouldnt be here? %d\n", (expected_type));
            if (type->value.type != expected_type){
                printf("%d", type->value.type);
                fprintf(stderr, "WRONG TYPE IN RETURN\n");
                exit(1);
            }
        }
        different_parse = 0;
        
    }
    
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
void rule_statement(item_ptr *in_stack, table_node_ptr *tree)
{
    DEBUG_RULE();
    DEBUG_OUT("U llllllllllllllllllllllllllllllllll here? \n");
    token_ptr next = get_next_token(in_stack);

    if (next->type == TOKEN_VAR_ID)
    {
        // <statement> -> var_id = <expression>;
        DEBUG_OUT("U hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh here? \n");

        assert_next_token(in_stack, TOKEN_ASSIGN);

        int value = parse_expression(in_stack);

        // Create symboltable entry if not already present
        if (sym_get_variable(*tree, next->value.string) == NULL)
        {
            // TODO: Infer type from value (requires PSA)
            // TODO-CHECK: Nullable by default?
            variable_ptr variable = variable_create(TYPE_INT, true);
            *tree = sym_insert(*tree, next->value.string, NULL, variable);
        }

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
            DEBUG_OUT("in while");
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

            assert_next_token(in_stack, TOKEN_ID);

            assert_next_token(in_stack, TOKEN_L_PAREN);
            rule_argument_list_typ(in_stack, tree);
            assert_next_token(in_stack, TOKEN_R_PAREN);


            assert_next_token(in_stack, TOKEN_COLON);
            token_ptr type_ = assert_next_token_get(in_stack, TOKEN_TYPE);
            if (type_->value.integer != TYPE_VOID){
                expect_return = 1;
                expected_type = type_->value.integer;
            } 
            assert_next_token(in_stack, TOKEN_LC_BRACKET);
            rule_statement_list(in_stack, tree);
            if (expect_return == 1){
                fprintf(stderr, "return expected not given.\n");
                exit(1); 
            }
            assert_next_token(in_stack, TOKEN_RC_BRACKET);
            break;
            //assert_next_token(in_stack, TOKEN_L_PAREN);

            //int value = parse_expression(in_stack);
           // rule_argument_list(in_stack, tree);

           // assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

           // DEBUG_OUTF("function (%d)", value);

           // rule_statement_list(in_stack, tree);

           // assert_next_token(in_stack, TOKEN_RC_BRACKET);

           // DEBUG_OUT("end function");

        case KEYWORD_RETURN:
            if (expect_return == 0){
                // no idea if it has the correct return exit code
                expected_type = -1;
            }
            different_parse = 1;
            expect_return = 0;
            DEBUG_OUT("U here hjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj? \n");
            value = parse_expression(in_stack);
            assert_next_token(in_stack, TOKEN_SEMICOLON);
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
void rule_statement_list(item_ptr *in_stack, table_node_ptr *tree)
{
    DEBUG_RULE();
    DEBUG_OUT("rule_statement_list");

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

void rule_argument_list_typ(item_ptr *in_stack, table_node_ptr *tree){
    DEBUG_OUT("rule_argument_list_typ");
    token_ptr next = peek_top(in_stack);
    if (next->type == TOKEN_TYPE){
        assert_next_token(in_stack, TOKEN_TYPE);
        assert_next_token(in_stack, TOKEN_VAR_ID);
        rule_argument_next_typ(in_stack, tree);
    }

}


void rule_argument_next_typ(item_ptr *in_stack, table_node_ptr *tree){
    DEBUG_OUT("rule_argument_next_type");
    token_ptr next = peek_top(in_stack);
    if (next->type ==TOKEN_COMMA){
        assert_next_token(in_stack, TOKEN_COMMA);
        assert_next_token(in_stack, TOKEN_TYPE);
        assert_next_token(in_stack, TOKEN_VAR_ID);
        rule_argument_next_typ(in_stack, tree);
    }
}


// <argument-list> -> type <expression> <argument-next>
// <argument-list> -> <expression> <argument-next>
// <argument-list> -> ε
//void rule_argument_list(item_ptr *in_stack, table_node_ptr tree)
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
void rule_prog(item_ptr *in_stack, table_node_ptr *tree)
{
   DEBUG_RULE();

    // TODO: Declare

    assert_next_token(in_stack, TOKEN_OPENING_TAG);

    token_ptr php = assert_next_token_get(in_stack, TOKEN_ID);

    if (strcmp(php->value.string, "php") != 0)
    {
        fprintf(stderr, "Wrong prolog.\n");
        exit(1); // TODO: Correct code.
    }

    token_dispose(php);
    // for now
    assert_next_token(in_stack, TOKEN_ID);
    assert_next_token(in_stack, TOKEN_L_PAREN);
    assert_next_token(in_stack, TOKEN_ID);
    assert_next_token(in_stack, TOKEN_ASSIGN);
    assert_next_token(in_stack, TOKEN_CONST_INT);
    assert_next_token(in_stack, TOKEN_R_PAREN);
    rule_statement_list(in_stack, tree);

    token_ptr closing = peek_top(in_stack);

    // Closing tag optional
    if (closing != NULL)
    {
        assert_next_token(in_stack, TOKEN_CLOSING_TAG);
    }
    
}

void rule_prog_end(item_ptr *in_stack, table_node_ptr *tree){
    token_ptr next = peek_top(in_stack);
    if (next->type == TOKEN_NULLABLE){
        assert_next_token(in_stack, TOKEN_NULLABLE);
        assert_next_token(in_stack, TOKEN_MORE);
    }
    

}
table_node_ptr parse(array_ptr tokens)
{
    table_node_ptr tree = sym_init();

    item_ptr in_stack = stack_init();

    // fill input stack from tokens
    element_ptr element = tokens->last;
    while (element != NULL)
    {
        symbol_ptr symbol = create_terminal(element->token);
        in_stack = stack_push(in_stack, symbol);
        element = element->prev;
    }

    rule_prog(&in_stack, &tree);

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
int prog_(item_ptr *in_stack, table_node_ptr tree){
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

int prog_end_(item_ptr *in_stack, table_node_ptr tree){
    token_ptr next = peek_top(in_stack);
    if (next->type == TOKEN_NULLABLE){
        if (!assert_next_token_(in_stack, TOKEN_NULLABLE)) error_func();
        if (!assert_next_token_(in_stack, TOKEN_MORE)) error_func();
    }
    return 1;
}
int statement_list_(item_ptr *in_stack, table_node_ptr tree){
    token_ptr next = peek_top(in_stack);
    // token id == meno funkcie ?
    if (((next->type == KEYWORD) || next->type == TOKEN_VAR_ID || next->type == TOKEN_ID)){
        statement_(in_stack, tree);
        statement_list_(in_stack, tree);
    }
    return 1;
}
int statement_(item_ptr *in_stack, table_node_ptr tree){
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
int argument_list_(item_ptr *in_stack, table_node_ptr tree);
int argument_next_(item_ptr *in_stack, table_node_ptr tree);
int argument_list_typ_(item_ptr *in_stack, table_node_ptr tree);
int argument_next_typ(item_ptr *in_stack, table_node_ptr tree);
int expression_(item_ptr *in_stack, table_node_ptr tree);
int expression_tail(item_ptr *in_stack, table_node_ptr tree);
*/