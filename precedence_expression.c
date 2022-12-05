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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char precedence_table[8][8] = {
    //   +-. ;   */ ;     (  ;     ) ;    i ;     <?(;) ; === !== ; ><<=>=
    {'>', '<', '<', '>', '<', '>', '>', '>'}, //+-.
    {'>', '>', '<', '>', '<', '>', '>', '>'}, //*/
    {'<', '<', 'n', '=', '<', 'x', '>', '<'}, // (
    {'>', '>', 'n', 'n', 'n', 'x', '>', '>'}, // )
    {'>', '>', 'n', '>', '<', '>', '>', '>'}, // i
    {'<', '<', '<', 'n', '<', 'n', '<', '<'}, // <? (;)
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

            // implicit conversions int -> float
            if (next->value_type == INTEGER && second_arg->value_type == FLOAT)
            {
                // convert second arg to float
                // can do this straight away, the second argument is on top of the stack
                // arg2
                // arg1

                INSTRUCTION_CMT(instr_buffer, "Second argument conversion");
                INSTRUCTION(instr_buffer, INSTR_INT2FLOATS);
                INSTRUCTION_CMT(instr_buffer, "End second argument conversion");
            }
            else if (next->value_type == FLOAT && second_arg->value_type == INTEGER)
            {
                // convert first arg to float
                // cannot do this straight away -> pop second arg into temp var, convert, push arg2 back
                // arg2
                // arg1

                // Create a new temporary frame
                INSTRUCTION_CMT(instr_buffer, "First argument conversion");
                INSTRUCTION(instr_buffer, INSTR_CREATE_FRAME);
                INSTRUCTION_OPS(instr_buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));
                INSTRUCTION_OPS(instr_buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));
                INSTRUCTION(instr_buffer, INSTR_INT2FLOATS);
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));
                INSTRUCTION_CMT(instr_buffer, "End first argument conversion");
            }
            else if (next->value_type == INTEGER && second_arg->value_type == INTEGER)
            {
                // both integers
                if (second_next->type == TOKEN_DIVIDE)
                {
                    // we're doing division, convert both

                    // arg2
                    // arg1

                    INSTRUCTION_CMT(instr_buffer, "Both argument conversion");

                    // convert arg2 on top
                    INSTRUCTION(instr_buffer, INSTR_INT2FLOATS);

                    // pop, convert arg1 under, push arg2 back
                    INSTRUCTION(instr_buffer, INSTR_CREATE_FRAME);
                    INSTRUCTION_OPS(instr_buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_arg2"));
                    INSTRUCTION_OPS(instr_buffer, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_arg2"));
                    INSTRUCTION(instr_buffer, INSTR_INT2FLOATS);
                    INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));

                    INSTRUCTION_CMT(instr_buffer, "End both argument conversion");
                }
            }

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
    if ((next_in->type == TOKEN_PLUS || next_in->type == TOKEN_MINUS || next_in->type == TOKEN_MULTIPLE || next_in->type == TOKEN_DIVIDE) && illegal_type == 1)
    {
        fprintf(stderr, "NUM OPERATORS WITH STRINGS NOT ALLOWED");

        // finnish exit number
        exit(100);
    }

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
        // not finnished
        // should prob just pop the brackets
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
        DEBUG(" I FINNISHED PARSING EXP\n");
    }
    else
    {
        expression_prec(in_stack, push_down_stack, table, instr);
    }
}