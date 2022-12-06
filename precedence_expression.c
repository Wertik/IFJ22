/*
 * Project: IFJ22 language compiler
 *
 * @author xsynak03 Maroš Synák
 * @author xotrad00 Martin Otradovec
 */

#include "stack.h"
#include "parser.h"
#include "utils.h"
#include "instruction.h"
#include "precedence_expression.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char precedence_table[8][8] = {
    //   +-. ;   */ ;     (  ;     ) ;    i ;     <?(;) ; === !== ; ><<=>=
    {'>', '<', '<', '>', '<', '>', '>', '>'}, //+-.
    {'>', '>', '<', '>', '<', '>', '>', '>'}, //*/
    {'<', '<', 'n', '=', '<', 'n', '>', '<'}, // ( // maybe worng n for ;
    {'>', '>', 'n', 'n', 'n', 'n', '>', '>'}, // ) 
    {'>', '>', 'n', '>', '<', '>', '>', '>'}, // i
    {'<', '<', '<', 's', '<', 'n', '<', '<'}, // <? (;) s is for speacial case replaced < hopefully will work
    {'<', '<', '<', '>', '<', '>', '>', '<'}, //===,!===
    {'<', '<', '<', '>', '<', '>', '>', '>'}, // <><=>=
};

token_ptr get_first_non_E(stack_ptr stack)
{
    DEBUG("GET FIRST NON E: ");
    token_ptr next = peek_top(stack);
    // token_value_t value;
    if (next->type == TOKEN_CONST_EXP)
    {
        DEBUG("CONST_EXP\n");

        token_ptr skipped = get_next_token(stack);
        next = peek_top(stack);

        // token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        // symbol_ptr symbol = create_terminal(E);

        symbol_ptr symbol = create_terminal(skipped);
        stack_push(stack, symbol);
    }

    DEBUG_TOKEN(next);
    return next;
}
int get_pos_in_t(token_ptr token)
{
    DEBUG("GET_POS_T:");
    DEBUG_TOKEN(token);

    switch (token->type)
    {
    case TOKEN_SEMICOLON:
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
    case TOKEN_VAR_ID:
    case TOKEN_STRING_LIT:
    case TOKEN_CONST_INT:
    case TOKEN_CONST_DOUBLE:
        DEBUG("4\n");
        return 4;
        break;
    case TOKEN_CONST_EXP:
        DEBUG(" FAILED\n");
        fprintf(stderr, "EXP SHOULD NOT END UP HERE");
        exit(FAIL_LEXICAL); // proper exit todo
        break;
    default:
        DEBUG(" RAN TO END\n");
        fprintf(stderr, "NON VALID TOKEN\n");
        exit(FAIL_LEXICAL); // proper exit todo
        break;
    }
    DEBUG(" RAN TO END\n");
    fprintf(stderr, "NON VALID TOKEN\n");
    exit(FAIL_LEXICAL); // proper exit todo
}

void conversion(instr_buffer_ptr instr_buffer, token_ptr arg1, token_type_t operator, token_ptr arg2)
{
    // Arithmetic conversions
    // int + int -> nothing
    // float + int -> float + float
    // int + float -> float + float
    // float + float -> nothing
    if (is_arithmetic(operator))
    {
        // implicit conversions int -> float
        if (arg1->value_type == INTEGER && arg2->value_type == FLOAT)
        {
            // convert second arg to float
            // can do this straight away, the second argument is on top of the stack
            // arg2
            // arg1

            INSTRUCTION_CMT(instr_buffer, "Second argument conversion");
            INSTRUCTION_CONV_ARG2_I2F(instr_buffer);
            INSTRUCTION_CMT(instr_buffer, "End second argument conversion");
        }
        else if (arg1->value_type == FLOAT && arg2->value_type == INTEGER)
        {
            // convert first arg to float
            // cannot do this straight away -> pop second arg into temp var, convert, push arg2 back
            // arg2
            // arg1

            INSTRUCTION_CMT(instr_buffer, "First argument conversion");
            INSTRUCTION_CONV_ARG1_I2F(instr_buffer);
            INSTRUCTION_CMT(instr_buffer, "End first argument conversion");
        }
        else if (arg1->value_type == INTEGER && arg2->value_type == INTEGER)
        {
            // both integers
            // int / int -> float / float
            if (arg2->type == TOKEN_DIVIDE)
            {
                // we're doing division, convert both

                // arg2
                // arg1

                INSTRUCTION_CMT(instr_buffer, "Both argument conversion");

                // convert arg2 on top
                INSTRUCTION_CONV_ARG2_I2F(instr_buffer);

                // pop, convert arg1 under, push arg2 back
                INSTRUCTION_CONV_ARG1_I2F(instr_buffer);

                INSTRUCTION_CMT(instr_buffer, "End both argument conversion");
            }
        }
    }
}

void perform_reduction(stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr_buffer)
{
    DEBUG_RULE();

    // TODO: Dispose of this token
    token_ptr next = get_next_token(push_down_stack);
    token_value_t value;

    if (next->type == TOKEN_CONST_INT)
    {
        // Perform const_int -> E

        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, next->value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        // Push the value onto the stack
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_int(next->value.integer));
        return;
    }
    if (next->type == TOKEN_CONST_DOUBLE)
    {
        // const_float -> E

        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        // Push the value onto the stack
        instr_buffer_append(instr_buffer, dyn_str("# Float value: %lf", next->value.float_value));
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_float(next->value.float_value));
        return;
    }
    if (next->type == TOKEN_STRING_LIT)
    {
        // string_lit -> E

        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        // Push the value onto the stack
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_str(next->value.string));
        return;
    }
    if (next->type == TOKEN_CONST_NULL)
    {
        // null -> E

        token_ptr E = token_create(TOKEN_CONST_EXP, INTEGER, value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, alloc_str("nil@nil"));
        return;
    }
    if (next->type == TOKEN_VAR_ID)
    {
        // var_id -> E

        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        // Push the value onto the stack
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_LOCAL, next->value.string));
        return;
    }
    if (next->type == TOKEN_CONST_EXP)
    {
        // TODO: Dispose
        token_ptr second_next = get_next_token(push_down_stack);

        // not a $
        if (second_next->type != TOKEN_SEMICOLON)
        {
            token_ptr second_arg = get_next_token(push_down_stack);

            DEBUG("Operator reduction: %s", token_type_to_name(second_next->type));

            conversion(instr_buffer, next, second_next->type, second_arg);

            // Generate instructions for operator
            switch (second_next->type)
            {
            case TOKEN_PLUS:
                INSTRUCTION(instr_buffer, INSTR_ADDS);
                break;
            case TOKEN_MINUS:
                INSTRUCTION(instr_buffer, INSTR_SUBS);
                break;
            case TOKEN_MULTIPLE:
                INSTRUCTION(instr_buffer, INSTR_MULS);
                break;
            case TOKEN_DIVIDE:
            {
                INSTRUCTION(instr_buffer, INSTR_DIVS);
                break;
            }
            case TOKEN_EQUAL:
            {
                INSTRUCTION_CMT(instr_buffer, "Equal check");

                EXPRESSION_EQS(instr_buffer);

                INSTRUCTION_CMT(instr_buffer, "End equal check");
                break;
            }
            case TOKEN_NOT_EQUAL:
            {
                // Type check
                INSTRUCTION_CMT(instr_buffer, "Not equal check");

                EXPRESSION_EQS(instr_buffer);

                // !(Types match && values match)
                INSTRUCTION(instr_buffer, INSTR_NOTS);

                INSTRUCTION_CMT(instr_buffer, "End not equal check");
                break;
            }
            default:
                fprintf(stderr, "Operator %s not supported in expresions.\n", token_type_to_name(second_next->type));
                exit(FAIL_INTERNAL);
            }

            // E <+ / - / * / /> E -> E
            token_ptr E = token_create(TOKEN_CONST_EXP, next->value_type, value);
            symbol_ptr symbol = create_terminal(E);
            stack_push(push_down_stack, symbol);
            return;
        }
    }

    fprintf(stderr, "Invalid expression. No reductions possible.\n");
    exit(FAIL_LEXICAL);
}

void perform_addition(stack_ptr in_stack, stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr)
{
    DEBUG_RULE();

    token_ptr next = get_next_token(in_stack);
    if (next->type == TOKEN_SEMICOLON)
    {
        symbol_ptr symbol = create_terminal(next);
        stack_push(in_stack, symbol);
    }
    else
    {
        symbol_ptr symbol = create_terminal(next);
        stack_push(push_down_stack, symbol);
    }
}

void expression_prec(stack_ptr in_stack, stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr)
{
    DEBUG_RULE();

    DEBUG_STACK_TOP(push_down_stack, 5);
    DEBUG_STACK_TOP(in_stack, 5);

    static int illegal_type = -1;

    // Next tokens
    token_ptr next_in = peek_top(in_stack);
    token_ptr next_push = get_first_non_E(push_down_stack);

    // Don't allow string concat with numbers
    if (next_in->type == TOKEN_EQUAL || next_in->type == TOKEN_NOT_EQUAL)
    {
        illegal_type = -1;
    }
    if (illegal_type == -1 && (next_in->type == TOKEN_CONST_INT || next_in->type == TOKEN_CONST_DOUBLE))
    {
        DEBUG("illegal type = 0\n");
        illegal_type = 0; // concatenate not allowed
    }

    // Don't allow arithmetic operators with strings
    if (illegal_type == -1 && next_in->type == TOKEN_STRING_LIT)
    {
        DEBUG("illegal type = 1\n");
        illegal_type = 1; // num operators not allowed not allowed
    }

    // Don't allow arithmetic operators with strings
    // if ((next_in->type == TOKEN_PLUS || next_in->type == TOKEN_MINUS || next_in->type == TOKEN_MULTIPLE || next_in->type == TOKEN_DIVIDE) && illegal_type == 1)
    //{
    //     fprintf(stderr, "NUM OPERATORS WITH STRINGS NOT ALLOWED");
    //
    //      // finnish exit number
    //     exit(100);
    //  }

    // Don't allow arithmetic operators with strings?
    if ((next_in->type == TOKEN_CONST_DOUBLE || next_in->type == TOKEN_CONST_INT) && illegal_type == 1)
    {
        fprintf(stderr, "NUM OPERATORS WITH STRINGS NOT ALLOWED");

        // finnish exit number
        exit(100);
    }

    // Don't allow arithmetic operators with strings?
    if ((next_in->type == TOKEN_STRING_LIT) && illegal_type == 0)
    {
        fprintf(stderr, "NUM OPERATORS WITH STRINGS NOT ALLOWED");

        // finnish exit number
        exit(100);
    }

    // Precedence table indexes
    int next_push_idx = get_pos_in_t(next_push);
    int next_in_idx = get_pos_in_t(next_in);

    if (precedence_table[next_push_idx][next_in_idx] == '>')
    {
        perform_reduction(push_down_stack, table, instr);
    }
    else if (precedence_table[next_push_idx][next_in_idx] == '<')
    {
        perform_addition(in_stack, push_down_stack, table, instr);
    }
    else if (precedence_table[next_push_idx][next_in_idx] == '=')
    {
        token_ptr top = get_next_token(push_down_stack);
        if (top->type == TOKEN_L_PAREN)
        {
            stack_pop(push_down_stack);
        }
        else if (peek_top(push_down_stack)->type == TOKEN_L_PAREN)
        {
            stack_pop(push_down_stack);
            symbol_ptr symbol = create_terminal(top);
            stack_push(push_down_stack, symbol);
        }
        else
        {
            fprintf(stderr, "ERROR IN PAREN PARSING");
            exit(1226);
        }

        stack_pop(in_stack);
        // not finnished
        // should prob just pop the brackets
    }

    else if (precedence_table[next_push_idx][next_in_idx] == 's')
    {
        DEBUG("SPECIAL CASE ________________________________________\n")
        token_value_t value;
        token_ptr skipped = get_next_token(in_stack);
        if (peek_top(in_stack)->type == TOKEN_LC_BRACKET){
            DEBUG("I am here \n");
            symbol_ptr symbol = create_terminal(skipped);
            stack_push(in_stack, symbol);

            token_ptr E = token_create(TOKEN_SEMICOLON, NONE, value);
            symbol_ptr sym = create_terminal(E);
            stack_push(in_stack, sym);
        }
        else{
            fprintf(stderr, "ERROR EXPRESSION NOT IN CORRECT ORDER");
            exit(FAIL_LEXICAL);
        }
    }

    else if (precedence_table[next_push_idx][next_in_idx] == 'n')
    {
        fprintf(stderr, "ERROR EXPRESSION NOT IN CORRECT ORDER");
        exit(FAIL_LEXICAL);
    }

    // not finnished
    bool finnish = ((get_first_non_E(push_down_stack)->type == TOKEN_SEMICOLON) && (peek_top(in_stack)->type == TOKEN_SEMICOLON));
    if (finnish == true)
    {
        token_ptr skipped = get_next_token(in_stack);
        if (!(peek_top(in_stack)->type == TOKEN_R_PAREN)){
            DEBUG(" NOT POPPING SEMICOLON\n");
            symbol_ptr symbol = create_terminal(skipped);
            stack_push(in_stack, symbol);
        }
        DEBUG(" I FINNISHED PARSING EXP\n");
    }
    else
    {
        expression_prec(in_stack, push_down_stack, table, instr);
    }
}