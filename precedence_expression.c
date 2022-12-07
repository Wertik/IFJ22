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
    // +-. ;*/ ; (  ; ) ; i ; <?(;) ; === !== ; ><<=>=
    {'>', '<', '<', '>', '<', '>', '>', '>'}, //+-.
    {'>', '>', '<', '>', '<', '>', '>', '>'}, //*/
    {'<', '<', 'n', '=', '<', 'n', '>', '<'}, // ( // maybe worng n for ;
    {'>', '>', 'n', 'n', 'n', 'n', '>', '>'}, // )
    {'>', '>', 'n', '>', 'n', '>', '>', '>'}, // i // changed < to n for i i
    {'<', '<', '<', 's', '<', 'n', '<', '<'}, // <? (;) s is for speacial case replaced < hopefully will work
    {'<', '<', '<', '>', '<', '>', '>', '<'}, //===,!===
    {'<', '<', '<', '>', '<', '>', '>', '>'}, // <><=>=
};

token_ptr get_first_non_E(stack_ptr stack)
{
    DEBUG("GET FIRST NON E: ");
    token_ptr next = peek_top(stack);
    if (next->type == TOKEN_CONST_EXP)
    {
        token_ptr skipped = get_next_token(stack);
        next = peek_top(stack);

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
    case TOKEN_CONST_NULL: // maybe wrong
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
        exit(FAIL_SYNTAX); // proper exit todo
        break;
    default:
        DEBUG(" RAN TO END\n");
        fprintf(stderr, "%d\n", token->type);
        fprintf(stderr, "NON VALID TOKEN\n");
        exit(FAIL_SYNTAX); // proper exit todo
        break;
    }
    DEBUG(" RAN TO END\n");
    fprintf(stderr, "NON VALID TOKEN\n");
    exit(FAIL_SYNTAX); // proper exit todo
}

void perform_reduction(stack_ptr push_down_stack, sym_table_ptr table, instr_buffer_ptr instr_buffer)
{
    DEBUG_RULE();

    token_ptr next = get_next_token(push_down_stack);
    token_value_t value;

    if (next->type == TOKEN_CONST_INT)
    {
        // Perform const_int -> E
        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, NONE, next->value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        // Push the value onto the stack
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_int(next->value.integer))
    }
    else if (next->type == TOKEN_CONST_DOUBLE)
    {
        // const_float -> E

        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, NONE, value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        // Push the value onto the stack
        instr_buffer_append(instr_buffer, dyn_str("# Float value: %lf", next->value.float_value));
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_float(next->value.float_value))
    }
    else if (next->type == TOKEN_STRING_LIT)
    {
        // string_lit -> E

        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, NONE, value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        // Push the value onto the stack
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_str(next->value.string))
    }
    else if (next->type == TOKEN_CONST_NULL)
    {
        // null -> E

        token_ptr E = token_create(TOKEN_CONST_EXP, NONE, value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, alloc_str("nil@nil"))
    }
    else if (next->type == TOKEN_VAR_ID)
    {
        // var_id -> E

        // am not sure about different types int string float
        token_ptr E = token_create(TOKEN_CONST_EXP, NONE, value);
        symbol_ptr symbol = create_terminal(E);
        stack_push(push_down_stack, symbol);

        // Add variable to symtable, so we generate a DEFVAR instruction

        variable_ptr var = sym_get_variable(table, next->value.string);

        if (var == NULL)
        {
            variable_ptr var = variable_create(next->value.string);
            sym_insert_var(table, var);
        }

        // Check if the variable is defined and the type
        // CREATEFRAME
        // DEFVAR TF@_type
        // TYPE TF@_type <var>
        // JUMPIFNEQ _4_type_check TF@_$x_type string@
        // # Variable not defined
        // EXIT int@5
        // LABEL _4_type_check
        // JUMPIFEQ _4_definition_check_success TF@_$x_type string@<type_formal>
        // # Type failure
        // EXIT int@4
        // LABEL _4_check_success
        int label_cnt = instr_buffer->len;

        INSTRUCTION_CMT(instr_buffer, "Variable definition check");

        INSTRUCTION(instr_buffer, INSTR_CREATE_FRAME);
        INSTRUCTION_OPS(instr_buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_var_type"));
        INSTRUCTION_OPS(instr_buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_var_type"), instr_var(FRAME_LOCAL, next->value.string));
        INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "_def_check_success"), instr_var(FRAME_TEMP, "_var_type"), instr_const_str(""));
        INSTRUCTION_OPS(instr_buffer, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_VAR_UNDEFINED));
        INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "_def_check_success"));

        INSTRUCTION_CMT(instr_buffer, "End variable definition check");

        // Push the value onto the stack
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_LOCAL, next->value.string))
    }
    else if (next->type == TOKEN_CONST_EXP)
    {
        token_ptr second_next = get_next_token(push_down_stack);

        // not a $
        if (second_next->type != TOKEN_SEMICOLON)
        {
            STACK_THROW(push_down_stack);

            DEBUG("Operator reduction: %s", token_type_to_name(second_next->type));

            // Generate instructions for operator
            switch (second_next->type)
            {
            case TOKEN_PLUS:
            {
                int label_cnt = instr_buffer->len;

                // temp frame help vars
                INSTRUCTION_CMT(instr_buffer, "Plus");
                EXPRESSION_TEMP(instr_buffer);

                // strings not allowed here
                EXPRESSION_STR_CHECK(instr_buffer, label_cnt);

                // Convert nil -> int
                EXPRESSION_ARGS_NIL2INT(instr_buffer, label_cnt);

                // Do a float conversion
                EXPRESSION_ARGS_INT2FLOAT(instr_buffer, label_cnt, "do_a_plus");

                INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "do_a_plus"));

                // do a +
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1"));
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));
                INSTRUCTION(instr_buffer, INSTR_ADDS);

                INSTRUCTION_CMT(instr_buffer, "End plus");
                break;
            }
            case TOKEN_MINUS:
            {
                int label_cnt = instr_buffer->len;

                // temp frame help vars
                INSTRUCTION_CMT(instr_buffer, "Minus");
                EXPRESSION_TEMP(instr_buffer);

                // strings not allowed here
                EXPRESSION_STR_CHECK(instr_buffer, label_cnt);

                // Convert nil -> int
                EXPRESSION_ARGS_NIL2INT(instr_buffer, label_cnt);

                // Do a float conversion
                EXPRESSION_ARGS_INT2FLOAT(instr_buffer, label_cnt, "do_a_minus");

                INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "do_a_minus"));

                // do a +
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1"));
                INSTRUCTION(instr_buffer, INSTR_SUBS);

                INSTRUCTION_CMT(instr_buffer, "End minus");
                break;
            }
            case TOKEN_MULTIPLE:
            {
                int label_cnt = instr_buffer->len;

                // temp frame help vars
                INSTRUCTION_CMT(instr_buffer, "Times");
                EXPRESSION_TEMP(instr_buffer);

                // strings not allowed here
                EXPRESSION_STR_CHECK(instr_buffer, label_cnt);

                // Convert nil -> int
                EXPRESSION_ARGS_NIL2INT(instr_buffer, label_cnt);

                // Do a float conversion
                EXPRESSION_ARGS_INT2FLOAT(instr_buffer, label_cnt, "do_a_times");

                INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "do_a_times"));

                // do a +
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1"));
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));
                INSTRUCTION(instr_buffer, INSTR_MULS);

                INSTRUCTION_CMT(instr_buffer, "End times");
                break;
            }
            case TOKEN_DIVIDE:
            {
                // for division, always convert both arguments to float

                int label_cnt = instr_buffer->len;

                // temp frame help vars
                INSTRUCTION_CMT(instr_buffer, "Division");
                EXPRESSION_TEMP(instr_buffer);

                // strings not allowed here
                EXPRESSION_STR_CHECK(instr_buffer, label_cnt);

                // Convert nil -> int
                EXPRESSION_ARGS_NIL2INT(instr_buffer, label_cnt);

                // Convert both to floats
                EXPRESSION_ARGS_2FLOAT(instr_buffer, label_cnt, "do_a_division");

                INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "do_a_division"));

                // do a +
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg2"));
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1"));
                INSTRUCTION(instr_buffer, INSTR_DIVS);

                INSTRUCTION_CMT(instr_buffer, "End division");
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
            case TOKEN_MORE:
                INSTRUCTION_CMT(instr_buffer, "MORE");

                EXPRESSION_MORE(instr_buffer);

                INSTRUCTION_CMT(instr_buffer, "END MORE CHECK");
                break;
            case TOKEN_MORE_EQUAL:
                INSTRUCTION_CMT(instr_buffer, "MORE_EQUAL");

                EXPRESSION_MORE_EQUAL(instr_buffer);

                INSTRUCTION_CMT(instr_buffer, "END MORE_EQUAL CHECK");
                break;
            case TOKEN_LESS:
                INSTRUCTION_CMT(instr_buffer, "LESS");

                EXPRESSION_LESS(instr_buffer);

                INSTRUCTION_CMT(instr_buffer, "END LESS CHECK");
                break;
            case TOKEN_LESS_EQUAL:
                INSTRUCTION_CMT(instr_buffer, "LESS_EQUAL");

                EXPRESSION_LESS_EQUAL(instr_buffer);

                INSTRUCTION_CMT(instr_buffer, "END LESS_EQUAL CHECK");
                break;
            case TOKEN_DOT:
            {
                int label_cnt = instr_buffer->len;

                INSTRUCTION_CMT(instr_buffer, "CONCATENATE");

                EXPRESSION_TEMP(instr_buffer);

                // don't allow anything other than a string
                INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_not_allowed"), instr_var(FRAME_TEMP, "_arg1_type"), instr_type_str(TYPE_INT));
                INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_not_allowed"), instr_var(FRAME_TEMP, "_arg2_type"), instr_type_str(TYPE_INT));
                INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_not_allowed"), instr_var(FRAME_TEMP, "_arg1_type"), instr_type_str(TYPE_FLOAT));
                INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_not_allowed"), instr_var(FRAME_TEMP, "_arg2_type"), instr_type_str(TYPE_FLOAT));
                INSTRUCTION_OPS(instr_buffer, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_allowed"));

                // throw a semantic error when not string
                INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_not_allowed"));
                INSTRUCTION_OPS(instr_buffer, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_EXPRE));

                // convert arg1 -> str
                INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_allowed"));
                INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "arg2_conv"), instr_var(FRAME_TEMP, "_arg1_type"), instr_const_str("nil"));
                INSTRUCTION_OPS(instr_buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg1"), instr_const_str(""));

                // convert arg2 -> str
                INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "arg2_conv"));
                INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "do_concat"), instr_var(FRAME_TEMP, "_arg2_type"), instr_const_str("nil"));
                INSTRUCTION_OPS(instr_buffer, INSTR_MOVE, 2, instr_var(FRAME_TEMP, "_arg2"), instr_const_str(""));

                INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "do_concat"));

                INSTRUCTION_OPS(instr_buffer, INSTR_CONCAT, 3, instr_var(FRAME_TEMP, "_arg1"), instr_var(FRAME_TEMP, "_arg2"), instr_var(FRAME_TEMP, "_arg1"))
                INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_arg1"));

                INSTRUCTION_CMT(instr_buffer, "END CONCATENATE");
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

            token_dispose(second_next);
        }
    }
    else
    {
        fprintf(stderr, "Invalid expression. No reductions possible.\n");
        exit(FAIL_SYNTAX); // changed err code
    }
    token_dispose(next);
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
    static int added_semicol = false;
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
    if ((next_in->type == TOKEN_PLUS || next_in->type == TOKEN_MINUS || next_in->type == TOKEN_MULTIPLE || next_in->type == TOKEN_DIVIDE) && illegal_type == 1)
    {
        fprintf(stderr, "NUM OPERATORS WITH STRINGS NOT ALLOWED 1");

        // finnish exit number
        exit(FAIL_SEMANTIC_EXPRE);
    }

    // Don't allow arithmetic operators with strings?
    if ((next_in->type == TOKEN_CONST_DOUBLE || next_in->type == TOKEN_CONST_INT) && illegal_type == 1)
    {
        fprintf(stderr, "NUM OPERATORS WITH STRINGS NOT ALLOWED");

        // finnish exit number
        exit(FAIL_SYNTAX);
    }

    // Don't allow arithmetic operators with strings?
    if ((next_in->type == TOKEN_STRING_LIT) && illegal_type == 0)
    {
        fprintf(stderr, "NUM OPERATORS WITH STRINGS NOT ALLOWED");

        // finnish exit number
        exit(FAIL_SYNTAX);
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
            STACK_THROW(push_down_stack);
            token_dispose(top);
        }
        else if (peek_top(push_down_stack)->type == TOKEN_L_PAREN)
        {
            STACK_THROW(push_down_stack);
            symbol_ptr symbol = create_terminal(top);
            stack_push(push_down_stack, symbol);
        }
        else
        {
            fprintf(stderr, "ERROR IN PAREN PARSING\n");
            exit(1226); // finnish exit code
        }

        STACK_THROW(in_stack);
        // not finnished
        // should prob just pop the brackets
    }

    else if (precedence_table[next_push_idx][next_in_idx] == 's')
    {
        DEBUG("SPECIAL CASE ________________________________________\n")
        token_value_t value;
        token_ptr skipped = get_next_token(in_stack);
        if (peek_top(in_stack)->type == TOKEN_LC_BRACKET)
        {
            DEBUG("I am here \n");
            symbol_ptr symbol = create_terminal(skipped);
            stack_push(in_stack, symbol);

            token_ptr E = token_create(TOKEN_SEMICOLON, NONE, value);
            symbol_ptr sym = create_terminal(E);
            stack_push(in_stack, sym);
            added_semicol = true;
        }
        else
        {
            fprintf(stderr, "ERROR EXPRESSION NOT IN CORRECT ORDER");
            exit(FAIL_SYNTAX); // correct codde?
        }
    }

    else if (precedence_table[next_push_idx][next_in_idx] == 'n')
    {
        fprintf(stderr, "ERROR EXPRESSION NOT IN CORRECT ORDER");
        exit(FAIL_SYNTAX); // correct code?
    }

    // not finnished
    DEBUG("AT FINNISH\n");
    bool finnish = ((get_first_non_E(push_down_stack)->type == TOKEN_SEMICOLON) && (peek_top(in_stack)->type == TOKEN_SEMICOLON));
    if (finnish == true)
    {
        token_ptr skipped = get_next_token(in_stack);

        if (!added_semicol)
        {
            symbol_ptr symbol = create_terminal(skipped);
            stack_push(in_stack, symbol);
        } else {
            token_dispose(skipped);
        }
        added_semicol = 0;
        illegal_type = -1;
        DEBUG(" I FINNISHED PARSING EXP\n");
    }
    else
    {
        expression_prec(in_stack, push_down_stack, table, instr);
    }
}