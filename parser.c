/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 * @author xbalek02 Miroslav Bálek
 * @author xdobes22 Kristán Dobeš
 * @author xsynak03 Maroš Synák
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "buffer.h"
#include "symtable.h"
#include "stack.h"
#include "utils.h"
#include "instruction.h"
#include "preparser.h"
#include "precedence_expression.h"

sym_table_ptr global_table = NULL;

token_ptr peek(stack_ptr stack, int n)
{
    item_ptr item = stack->top;

    for (int i = 0; i < n && item != NULL; i++)
    {
        item = item->next;
    }

    if (item == NULL)
    {
        return NULL;
    }

    symbol_ptr symbol = item->symbol;
    return symbol->token;
}

token_ptr peek_top(stack_ptr stack)
{
    item_ptr top = stack_top(stack);

    if (top == NULL)
    {
        return NULL;
    }
    symbol_ptr symbol = top->symbol;
    return symbol->token;
}

token_ptr peek_exact_type(stack_ptr stack, token_type_t token_type)
{
    token_ptr token = peek_top(stack);
    if (token == NULL)
    {
        return NULL;
    }
    return token->type == token_type ? token : NULL;
}

token_ptr get_next_token(stack_ptr stack)
{
    item_ptr item = stack_pop(stack);

    if (item == NULL)
    {
        return NULL;
    }

    symbol_ptr symbol = item->symbol;
    token_ptr token = symbol->token;

    DEBUG_TOKEN(token);
    DEBUG_STACK_TOP(stack, 2);

    free(symbol);
    free(item);
    return token;
}

token_ptr assert_next_token_get(stack_ptr stack, token_type_t token_type)
{
    token_ptr token = get_next_token(stack);

    ASSERT_TOKEN_TYPE(token, token_type);

    return token;
}

void assert_n_tokens(stack_ptr stack, int n, ...)
{
    va_list valist;
    va_start(valist, n);

    for (int i = 0; i < n; i++)
    {
        ASSERT_NEXT_TOKEN(stack, va_arg(valist, token_type_t));
    }

    va_end(valist);
}

// <expression> -> ... precedence
void rule_expression(stack_ptr stack, sym_table_ptr table, instr_buffer_ptr instr_buffer)
{
    stack_ptr push_down_stack = stack_init();

    // represents dollar
    token_value_t value;
    token_ptr dollar = token_create(TOKEN_SEMICOLON, NONE, value);
    symbol_ptr symbol = create_terminal(dollar);
    stack_push(push_down_stack, symbol);

    expression_prec(stack, push_down_stack, table, instr_buffer);
}

// <statement> -> var_id = <expression>;
// <statement> -> var_id = id(<arg-list>);
// <statement> -> return <expression>;
// <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}
// <statement> -> while (<expression>) {<statement-list>}
// <statement> -> function id(<argument-list>) {<statement-list>}
// <statement> -> id(<argument-list>);
// <statement> -> <expression>;
void rule_statement(stack_ptr stack, sym_table_ptr table, function_ptr function, instr_buffer_ptr instr)
{
    DEBUG_RULE();

    token_ptr next = peek_top(stack);
    token_ptr after_next = peek(stack, 1);

    // <statement> -> var_id = <expression>;
    if (next->type == TOKEN_VAR_ID && after_next != NULL && after_next->type == TOKEN_ASSIGN)
    {
        token_ptr var_id = get_next_token(stack);
        // throw away assign
        STACK_THROW(stack);

        variable_ptr var = sym_get_variable(table, var_id->value.string);

        if (var == NULL)
        {
            // default values for type, gets filled in later
            var = variable_create(var_id->value.string);
            sym_insert_var(table, var);

            // variable DEFVARs get generated at the end of parsing. We insert them at the start of the current buffer.
        }

        next = peek_top(stack);

        if (next->type == TOKEN_ID)
        {
            // <statement> -> var_id = id(<arg-list>);

            token_ptr func_id = get_next_token(stack);

            ASSERT_NEXT_TOKEN(stack, TOKEN_L_PAREN);

            function_ptr called_function = sym_get_function(global_table, func_id->value.string);

            // check if the function exists
            if (called_function == NULL)
            {
                fprintf(stderr, "Function %s not defined.\n", func_id->value.string);
                exit(FAIL_SEMANTIC_FUNC_DEF);
            }

            DEBUG_PSEUDO("%s = %s(...)", var_id->value.string, func_id->value.string);

            called_function->called = true;

            int argument_count = rule_argument_list(stack, table, called_function, instr, called_function->variadic);

            // Push count of arguments for variadic functions
            if (called_function->variadic)
            {
                INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_int(argument_count));
            }

            INSTRUCTION_CMT(instr, "Function call with assignment");

            // Call the function
            INSTRUCTION_OPS(instr, INSTR_CALL, 1, alloc_str(called_function->name));

            INSTRUCTION_CMT(instr, "Function return value check");

            if (called_function->return_type != TYPE_VOID)
            {
                int label_cnt = instr->len;

                // if non-void, check that the function has returned something
                INSTRUCTION(instr, INSTR_CREATE_FRAME);
                INSTRUCTION_OPS(instr, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_has_retval"));
                INSTRUCTION_OPS(instr, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_has_retval"));
                INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "_has_retval_success"), instr_var(FRAME_TEMP, "_has_retval"), instr_const_bool(true));

                // Throw error 4
                INSTRUCTION_OPS(instr, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_BAD_ARGS));

                INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "_has_retval_success"));

                FUNCTION_RETURN_TYPE_CHECK(instr, function, called_function);
            }
            else
            {
                int label_cnt = instr->len;

                // if void, check that the function hasn't returned anything
                INSTRUCTION(instr, INSTR_CREATE_FRAME);
                INSTRUCTION_OPS(instr, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_has_retval"));
                INSTRUCTION_OPS(instr, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_has_retval"));
                INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "_has_retval_success"), instr_var(FRAME_TEMP, "_has_retval"), instr_const_bool(false));

                // Throw error 4
                INSTRUCTION_OPS(instr, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_BAD_ARGS));

                INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "_has_retval_success"));
            }

            INSTRUCTION_CMT(instr, "End function return value check");

            // Pop the return value from the stack into the variable
            INSTRUCTION_OPS(instr, INSTR_POPS, 1, instr_var(FRAME_LOCAL, var_id->value.string));

            INSTRUCTION_CMT(instr, "End function call with assignment");

            ASSERT_NEXT_TOKEN(stack, TOKEN_R_PAREN);
        }
        else
        {
            // <statement> -> var_id = <expression>;

            // parse r-side
            rule_expression(stack, table, instr);

            // Get the expression result from stack
            INSTRUCTION_OPS(instr, INSTR_POPS, 1, instr_var(FRAME_LOCAL, var->name));
        }

        DEBUG_PSEUDO("Assign to %s", var_id->value.string);

        ASSERT_NEXT_TOKEN(stack, TOKEN_SEMICOLON);
        token_dispose(var_id);
    }
    else if (next->type == TOKEN_ID)
    {
        // <statement> -> id(<argument-list>);

        // Check that the function exists.
        token_ptr func_id = assert_next_token_get(stack, TOKEN_ID);

        ASSERT_NEXT_TOKEN(stack, TOKEN_L_PAREN);

        function_ptr called_function = sym_get_function(global_table, func_id->value.string);

        if (called_function == NULL)
        {
            fprintf(stderr, "Function %s not defined.\n", func_id->value.string);
            exit(FAIL_SEMANTIC_FUNC_DEF);
        }

        DEBUG_PSEUDO("%s(...)", func_id->value.string);

        called_function->called = true;

        INSTRUCTION_CMT(instr, "Function call");

        int argument_count = rule_argument_list(stack, table, called_function, instr, called_function->variadic);

        // Push count of arguments for variadic functions
        if (called_function->variadic)
        {
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_int(argument_count));
        }

        // Function call code
        INSTRUCTION_OPS(instr, INSTR_CALL, 1, alloc_str(called_function->name));

        INSTRUCTION_CMT(instr, "Function return value check");

        if (called_function->return_type != TYPE_VOID)
        {
            int label_cnt = instr->len;

            // if non-void, check that the function has returned something
            INSTRUCTION(instr, INSTR_CREATE_FRAME);
            INSTRUCTION_OPS(instr, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_has_retval"));
            INSTRUCTION_OPS(instr, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_has_retval"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "_has_retval_success"), instr_var(FRAME_TEMP, "_has_retval"), instr_const_bool(true));

            // Throw error 4
            INSTRUCTION_OPS(instr, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_BAD_ARGS));

            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "_has_retval_success"));

            FUNCTION_RETURN_TYPE_CHECK(instr, function, called_function);
        }
        else
        {
            int label_cnt = instr->len;

            // if void, check that the function hasn't returned anything
            INSTRUCTION(instr, INSTR_CREATE_FRAME);
            INSTRUCTION_OPS(instr, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_has_retval"));
            INSTRUCTION_OPS(instr, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_has_retval"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "_has_retval_success"), instr_var(FRAME_TEMP, "_has_retval"), instr_const_bool(false));

            // Throw error 4
            INSTRUCTION_OPS(instr, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_BAD_ARGS));

            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "_has_retval_success"));
        }

        INSTRUCTION_CMT(instr, "End function return value check");

        // Clear the stack after a function call in case the function returned a value to the stack.
        INSTRUCTION(instr, INSTR_CLEARS);

        INSTRUCTION_CMT(instr, "End function call");

        ASSERT_NEXT_TOKEN(stack, TOKEN_R_PAREN);
        ASSERT_NEXT_TOKEN(stack, TOKEN_SEMICOLON);
        token_dispose(func_id);
    }
    else if (next->type == TOKEN_KEYWORD)
    {
        // if / function / while

        next = assert_next_token_get(stack, TOKEN_KEYWORD);

        switch (next->value.keyword)
        {
        case KEYWORD_IF:
        {
            // <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}

            // if (<expression>) {<statement-list>}

            ASSERT_NEXT_TOKEN(stack, TOKEN_L_PAREN);

            int label_cnt = instr->len;

            INSTRUCTION_CMT(instr, "If expression");

            rule_expression(stack, table, instr);

            ASSERT_NEXT_TOKEN(stack, TOKEN_R_PAREN);
            ASSERT_NEXT_TOKEN(stack, TOKEN_LC_BRACKET);

            DEBUG_PSEUDO("if (...)");

            INSTRUCTION_CMT(instr, "If condition check");

            // Compare expression result to true and jump to else if not true.
            // null, emptry string, 0, "0" -> false
            INSTRUCTION(instr, INSTR_CREATE_FRAME);
            INSTRUCTION_OPS(instr, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_cond"));
            INSTRUCTION_OPS(instr, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_cond"));
            INSTRUCTION_OPS(instr, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_condtype"), instr_var(FRAME_TEMP, "_cond"));

            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_else"), instr_var(FRAME_TEMP, "_cond"), alloc_str("nil@nil"));

            // Compare boolean values
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_str("bool"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_cond_int"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_else"), instr_var(FRAME_TEMP, "_cond"), instr_const_bool(false));
            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_if"));

            // Compare int values
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_cond_int"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_str("int"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_cond_string"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_else"), instr_var(FRAME_TEMP, "_cond"), instr_const_int(0));
            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_if"));

            // Compare string values
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_cond_string"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_str("string"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_cond_float"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_else"), instr_var(FRAME_TEMP, "_cond"), instr_const_str(""));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_else"), instr_var(FRAME_TEMP, "_cond"), instr_const_str("0"));
            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_if"));

            // Compare float values
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_cond_float"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_str("float"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_if"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_else"), instr_var(FRAME_TEMP, "_cond"), instr_const_float(0.0));

            INSTRUCTION_CMT(instr, "If body");
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_if"));

            rule_statement_list(stack, table, function, instr);

            ASSERT_NEXT_TOKEN(stack, TOKEN_RC_BRACKET);

            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_after_else"));

            DEBUG_PSEUDO("end if");

            // else { <statement-list> }

            ASSERT_KEYWORD(stack, KEYWORD_ELSE);

            ASSERT_NEXT_TOKEN(stack, TOKEN_LC_BRACKET);

            DEBUG_PSEUDO("else");

            INSTRUCTION_CMT(instr, "Else body");
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_begin_else"));

            rule_statement_list(stack, table, function, instr);

            ASSERT_NEXT_TOKEN(stack, TOKEN_RC_BRACKET);

            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "if_after_else"));
            INSTRUCTION_CMT(instr, "End else body");

            DEBUG_PSEUDO("end else");
            break;
        }
        case KEYWORD_WHILE:
        {
            // <statement> -> while (<expression>) {<statement-list>}
            ASSERT_NEXT_TOKEN(stack, TOKEN_L_PAREN);
            int label_cnt = instr->len;

            INSTRUCTION(instr, INSTR_CREATE_FRAME);

            // label to check condition
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_condition"));

            // condition generation
            rule_expression(stack, table, instr);
            ASSERT_NEXT_TOKEN(stack, TOKEN_R_PAREN);
            ASSERT_NEXT_TOKEN(stack, TOKEN_LC_BRACKET);
            // assert_n_tokens(stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_PSEUDO("while (...)");

            INSTRUCTION_CMT(instr, "While condition check");

            // Compare expression result to true and jump to else if not true.
            // null, emptry string, 0, "0" -> false
            // while (
            INSTRUCTION_OPS(instr, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_cond"));
            INSTRUCTION_OPS(instr, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_POPS, 1, instr_var(FRAME_TEMP, "_cond"));
            INSTRUCTION_OPS(instr, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_condtype"), instr_var(FRAME_TEMP, "_cond"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_exit"), instr_var(FRAME_TEMP, "_cond"), alloc_str("nil@nil"));

            // Compare boolean values
            // Compare boolean values
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_str("bool"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_cond_int"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_exit"), instr_var(FRAME_TEMP, "_cond"), instr_const_bool(false));
            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_begin"));

            // Compare int values
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_cond_int"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_str("int"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_cond_string"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_exit"), instr_var(FRAME_TEMP, "_cond"), instr_const_int(0));
            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_begin"));

            // Compare string values
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_cond_string"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_str("string"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_cond_float"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_exit"), instr_var(FRAME_TEMP, "_cond"), instr_const_str(""));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_exit"), instr_var(FRAME_TEMP, "_cond"), instr_const_str("0"));
            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_begin"));

            // Compare float values
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_cond_float"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_var(FRAME_TEMP, "_condtype"));
            INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_str("float"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFNEQS, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_begin"));
            INSTRUCTION_OPS(instr, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_exit"), instr_var(FRAME_TEMP, "_cond"), instr_const_float(0.0));
            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_begin"));

            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_begin"));

            // ){
            INSTRUCTION_CMT(instr, "While body");
            rule_statement_list(stack, table, function, instr);
            INSTRUCTION_CMT(instr, "End while body");
            // }
            INSTRUCTION_CMT(instr, "Jump to start to check condition");
            INSTRUCTION_OPS(instr, INSTR_JUMP, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_condition"));
            INSTRUCTION_CMT(instr, "Condition is false leave the cycle");
            INSTRUCTION_OPS(instr, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr, label_cnt, "while_exit"));

            ASSERT_NEXT_TOKEN(stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end while");
            break;
        }
        case KEYWORD_FUNCTION:
        {
            // <statement> -> function id(<argument-list>) : type {<statement-list>}

            // Save function to symtable
            token_ptr function_id = assert_next_token_get(stack, TOKEN_ID);

            // Get function from global table
            // We already should have parameters pre-parsed.

            function_ptr function = sym_get_function(global_table, function_id->value.string);

            // This shouldn't happen
            if (function == NULL)
            {
                fprintf(stderr, "Function %s not preparsed. Something went wrong.\n", function_id->value.string);
                exit(FAIL_INTERNAL);
            }

            DEBUG_PSEUDO("function %s(...)", function_id->value.string);

            // Parse parameters

            ASSERT_NEXT_TOKEN(stack, TOKEN_L_PAREN);

            // Throw away the parameter tokens
            rule_parameter_list(stack, function);

            ASSERT_NEXT_TOKEN(stack, TOKEN_R_PAREN);

            ASSERT_NEXT_TOKEN(stack, TOKEN_COLON);

            token_ptr potentional_nullable = peek_top(stack);

            if (potentional_nullable->type == TOKEN_NULLABLE)
            {
                STACK_THROW(stack);
                function->return_type_nullable = true;
            }

            ASSERT_NEXT_TOKEN(stack, TOKEN_TYPE);

            ASSERT_NEXT_TOKEN(stack, TOKEN_LC_BRACKET);

            // Generate function parameters code

            for (int i = 0; i < function->parameter_count; i++)
            {
                parameter_t parameter = function->parameters[i];
                INSTRUCTION_OPS(function->instr_buffer, INSTR_POPS, 1, instr_var(FRAME_LOCAL, parameter.name));
            }

            // Function statement list

            rule_statement_list(stack, function == NULL ? table : function->symtable, function, function == NULL ? instr : function->instr_buffer);

            if (function->return_type != TYPE_VOID && function->has_return == false)
            {
                fprintf(stderr, "Missing return statement for non-void function %s.\n", function_id->value.string);
                exit(FAIL_SEMANTIC_BAD_ARGS);
            }

            // Generate implicit return

            // Returns nothing
            INSTRUCTION_OPS(function->instr_buffer, INSTR_PUSHS, 1, instr_const_bool(false));
            INSTRUCTION(function->instr_buffer, INSTR_POP_FRAME);
            FUNCTION_RETURN(function->instr_buffer);

            ASSERT_NEXT_TOKEN(stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end function %s", function_id->value.string);

            token_dispose(function_id);
            break;
        }
        case KEYWORD_RETURN:
        {
            // Global return
            if (function == NULL)
            {
                // Can return a value or not.
                token_ptr next = peek_top(stack);

                // not a semicolon, has to be an expression or constant
                if (next->type != TOKEN_SEMICOLON)
                {
                    rule_expression(stack, table, instr);
                }

                DEBUG_PSEUDO("global return ...;");
                ASSERT_NEXT_TOKEN(stack, TOKEN_SEMICOLON);
            }
            else
            {
                token_ptr next = peek_top(stack);

                // Function return

                if (function->return_type == TYPE_VOID)
                {

                    if (next->type != TOKEN_SEMICOLON)
                    {
                        fprintf(stderr, "Void function %s shouldn't return a value.\n", function->name);
                        exit(FAIL_SEMANTIC_INVALID_RETURN_COUNT);
                    }

                    // Push null as the result of a void function
                    INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, alloc_str("nil@nil"));

                    // Indicates that no value has been returned
                    INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_bool(false));

                    DEBUG_PSEUDO("return;");
                }
                else
                {

                    if (next->type == TOKEN_SEMICOLON)
                    {
                        fprintf(stderr, "Missing return value in non-void function %s.\n", function->name);
                        exit(FAIL_SEMANTIC_INVALID_RETURN_COUNT);
                    }

                    // The return value should be pushed onto the stack in expression
                    rule_expression(stack, table, instr);

                    // Indicates that a value has been returned
                    INSTRUCTION_OPS(instr, INSTR_PUSHS, 1, instr_const_bool(true));

                    DEBUG_PSEUDO("return ...;");
                }
                function->has_return = true;
                ASSERT_NEXT_TOKEN(stack, TOKEN_SEMICOLON);

                INSTRUCTION(instr, INSTR_POP_FRAME);
                INSTRUCTION(instr, INSTR_RETURN);
            }
            break;
        }
        default:
            fprintf(stderr, "Invalid keyword in statement.\n");
            exit(FAIL_SYNTAX);
        }
        token_dispose(next);
    }
    else
    {
        // just let it try to parse an expression here
        rule_expression(stack, table, instr);

        ASSERT_NEXT_TOKEN(stack, TOKEN_SEMICOLON);
    }
}

// <statement-list> -> eps
// <statement-list> -> <statement><statement-list>
void rule_statement_list(stack_ptr stack, sym_table_ptr table, function_ptr function, instr_buffer_ptr instr)
{
    DEBUG_RULE();

    token_ptr next = peek_top(stack);

    // Nothing else to parse
    if (next == NULL)
    {
        // <statement-list> -> eps
        return;
    }

    if (is_one_of(next, 7, TOKEN_KEYWORD,
                  TOKEN_VAR_ID,
                  TOKEN_ID,
                  TOKEN_CONST_INT,
                  TOKEN_CONST_DOUBLE,
                  TOKEN_STRING_LIT,
                  TOKEN_L_PAREN)) // dont know man
    {
        // <statement-list> -> <statement><statement-list>
        rule_statement(stack, table, function, instr);

        rule_statement_list(stack, table, function, instr);
    }
    else
    {
        // <statement-list> -> eps
        return;
    }
}

// <par> -> type var_id
void rule_parameter(stack_ptr stack, function_ptr function)
{
    if (peek_top(stack)->type == TOKEN_NULLABLE)
    {
        STACK_THROW(stack);
    }

    ASSERT_NEXT_TOKEN(stack, TOKEN_TYPE);
    ASSERT_NEXT_TOKEN(stack, TOKEN_VAR_ID);
}

// <par-list> -> eps
// <par-list> -> <par> <par-next>
void rule_parameter_list(stack_ptr stack, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = peek_top(stack);

    if (next->type == TOKEN_TYPE || next->type == TOKEN_NULLABLE)
    {
        // <par-list> -> <par> <par-next>
        rule_parameter(stack, function);
        rule_parameter_next(stack, function);
    }
    else
    {
        // <arg-list> -> eps
        return;
    }
}

// <par-next> -> eps
// <par-next> -> , <par> <par-next>
void rule_parameter_next(stack_ptr stack, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = peek_top(stack);

    if (next->type == TOKEN_COMMA)
    {
        // <par-next> -> , <par> <par-next>
        STACK_THROW(stack);

        rule_parameter_list(stack, function);
    }
    else
    {
        // <arg-next> -> eps
        return;
    }
}

// <arg> -> var_id
// <arg> -> const_int
// <arg> -> string_lit
// <arg> -> const_float
void rule_argument(stack_ptr stack, sym_table_ptr table, parameter_t *parameter, instr_buffer_ptr instr_buffer)
{
    token_ptr next = get_next_token(stack);

    switch (next->type)
    {
    case TOKEN_VAR_ID:
    {
        variable_ptr var = sym_get_variable(table, next->value.string);

        if (var == NULL)
        {
            fprintf(stderr, "Unknown variable %s.\n", next->value.string);
            exit(FAIL_SEMANTIC_VAR_UNDEFINED);
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

        INSTRUCTION_CMT(instr_buffer, "Argument type check");

        INSTRUCTION(instr_buffer, INSTR_CREATE_FRAME);
        INSTRUCTION_OPS(instr_buffer, INSTR_DEFVAR, 1, instr_var(FRAME_TEMP, "_var_type"));
        INSTRUCTION_OPS(instr_buffer, INSTR_TYPE, 2, instr_var(FRAME_TEMP, "_var_type"), instr_var(FRAME_LOCAL, var->name));
        INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFNEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "def_check_success"), instr_var(FRAME_TEMP, "_var_type"), instr_const_str(""));
        INSTRUCTION_OPS(instr_buffer, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_VAR_UNDEFINED));
        INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "def_check_success"));

        // don't do type checks for variadic arguments
        // don't do type checks for TYPE_ANY parameters
        if (parameter != NULL && parameter->type != TYPE_ANY)
        {
            INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_check_success"), instr_var(FRAME_TEMP, "_var_type"), instr_type_str(parameter->type));

            // Allow nullable
            if (parameter->type_nullable)
            {
                INSTRUCTION_OPS(instr_buffer, INSTR_JUMPIFEQ, 3, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_check_success"), instr_var(FRAME_TEMP, "_var_type"), alloc_str("nil@nil"));
            }

            INSTRUCTION_OPS(instr_buffer, INSTR_EXIT, 1, instr_const_int(FAIL_SEMANTIC_BAD_ARGS));
            INSTRUCTION_OPS(instr_buffer, INSTR_LABEL, 1, INSTRUCTION_GEN_CTX_LABEL(instr_buffer, label_cnt, "type_check_success"));
        }
        else
        {
            INSTRUCTION_CMT(instr_buffer, "Variadic, no type check");
        }
        INSTRUCTION_CMT(instr_buffer, "End argument type check");

        // Push the variable onto the stack
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_var(FRAME_LOCAL, next->value.string));
        break;
    }
    case TOKEN_CONST_INT:
    {
        if (parameter != NULL && parameter->type != TYPE_INT && parameter->type != TYPE_ANY)
        {
            fprintf(stderr, "Bad argument type for %s. Expected %s but got %s.\n", parameter->name, type_to_name(parameter->type), "TYPE_INT");
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }

        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_int(next->value.integer));
        break;
    }
    case TOKEN_CONST_DOUBLE:
    {
        if (parameter != NULL && parameter->type != TYPE_FLOAT && parameter->type != TYPE_ANY)
        {
            fprintf(stderr, "Bad argument type for %s. Expected %s but got %s.\n", parameter->name, type_to_name(parameter->type), "TYPE_FLOAT");
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_float(next->value.float_value));
        break;
    }
    case TOKEN_STRING_LIT:
    {

        if (parameter != NULL && parameter->type != TYPE_STRING && parameter->type != TYPE_ANY)
        {
            fprintf(stderr, "Bad argument type for %s. Expected %s but got %s.\n", parameter->name, type_to_name(parameter->type), "TYPE_STRING");
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, instr_const_str(next->value.string));
        break;
    }
    case TOKEN_CONST_NULL:
    {
        if (parameter != NULL && parameter->type != TYPE_ANY && parameter->type_nullable == false)
        {
            fprintf(stderr, "Bad argument type for %s. Expected %s but got %s.\n", parameter->name, type_to_name(parameter->type), "TYPE_STRING");
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }
        INSTRUCTION_OPS(instr_buffer, INSTR_PUSHS, 1, alloc_str("nil@nil"));
        break;
    }
    default:
        fprintf(stderr, "Invalid token %s in function argument.\n", token_type_to_name(next->type));
        exit(FAIL_SYNTAX);
    }

    token_dispose(next);
}

// <arg-list> -> eps
// <arg-list> -> <arg> <arg-next>
int rule_argument_list(stack_ptr stack, sym_table_ptr table, function_ptr function, instr_buffer_ptr instr_buffer, bool variadic)
{
    DEBUG_RULE();

    token_ptr next = peek_top(stack);

    // Ignore argument count checks with variadic functions

    // <arg-list> -> eps
    if (!is_one_of(next, 5, TOKEN_VAR_ID, TOKEN_CONST_INT, TOKEN_CONST_DOUBLE, TOKEN_STRING_LIT, TOKEN_CONST_NULL))
    {
        // expected more than zero argumnets
        if (!variadic && function->parameter_count > 0)
        {
            fprintf(stderr, "Not enough arguments for function. Expected %d but got %d.\n", function->parameter_count, 0);
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }
        return 0;
    }

    parameter_t *expected_parameter = variadic ? NULL : &(function->parameters[0]);

    // Create a side buffer, save push instruction there, append after the recursive call.
    // We do this to push arguments from last to first.
    // hello(1, 2) -> PUSHS 2, PUSHS 1

    // Prefix the buffer so no collisions occur on multiple arguments
    // This is ugly, running out of time.
    char *prefix = expected_parameter == NULL ? dyn_str("%s_call_%s_%d_arg_%d",
                                                        instr_buffer->prefix == NULL ? "main" : instr_buffer->prefix,
                                                        function->name,
                                                        instr_buffer->len,
                                                        0)
                                              : dyn_str("%s_call_%s_%d_arg_%s",
                                                        instr_buffer->prefix == NULL ? "main" : instr_buffer->prefix,
                                                        function->name,
                                                        instr_buffer->len,
                                                        expected_parameter->name);
    instr_buffer_ptr arg_buffer = instr_buffer_init(prefix);
    free(prefix);

    rule_argument(stack, table, expected_parameter, arg_buffer);

    int argument_count = rule_argument_next(stack, table, function, instr_buffer, 1, variadic);

    if (!variadic && argument_count > function->parameter_count)
    {
        fprintf(stderr, "Too many parameters for function. Expected %d but got %d.\n", function->parameter_count, argument_count);
        exit(FAIL_SEMANTIC_BAD_ARGS);
    }

    if (!variadic && function->parameter_count > argument_count)
    {
        fprintf(stderr, "Not enough arguments for function. Expected %d but got %d.\n", function->parameter_count, argument_count);
        exit(FAIL_SEMANTIC_BAD_ARGS);
    }

    // append argument instructions
    for (int i = 0; i < arg_buffer->len; i++)
    {
        instr_buffer_append(instr_buffer, arg_buffer->instructions[i]);
    }

    // only free certain parts to retain instructions
    free(arg_buffer->prefix);
    free(arg_buffer);

    return argument_count;
}

// <arg-next> -> eps
// <arg-next> -> , <arg> <arg-next>
int rule_argument_next(stack_ptr stack, sym_table_ptr table, function_ptr function, instr_buffer_ptr instr_buffer, int argument_count, bool variadic)
{
    DEBUG_RULE();

    token_ptr next = peek_top(stack);

    if (next->type != TOKEN_COMMA)
    {
        // <arg-next> -> eps
        return argument_count;
    }

    STACK_THROW(stack);

    argument_count += 1;

    next = peek_top(stack);

    if (!is_one_of(next, 5, TOKEN_VAR_ID, TOKEN_CONST_INT, TOKEN_CONST_DOUBLE, TOKEN_STRING_LIT, TOKEN_CONST_NULL))
    {
        fprintf(stderr, "Expected an argument.\n");
        exit(FAIL_SYNTAX);
    }

    if (!variadic && argument_count > function->parameter_count)
    {
        fprintf(stderr, "Too many arguments for function. Expected %d but got %d.\n", function->parameter_count, argument_count);
        exit(FAIL_SEMANTIC_BAD_ARGS);
    }

    parameter_t *expected_parameter = variadic ? NULL : &(function->parameters[argument_count - 1]);

    // Create a side buffer, save push instruction there, append after the recursive call.
    // We do this to push arguments from last to first.
    // hello(1, 2) -> PUSHS 2, PUSHS 1

    // Create a unique prefix...
    char *prefix = expected_parameter == NULL ? dyn_str("%s_call_%s_%d_arg_%d",
                                                        instr_buffer->prefix == NULL ? "main" : instr_buffer->prefix,
                                                        function->name,
                                                        instr_buffer->len,
                                                        argument_count - 1)
                                              : dyn_str("%s_call_%s_%d_arg_%s",
                                                        instr_buffer->prefix == NULL ? "main" : instr_buffer->prefix,
                                                        function->name,
                                                        instr_buffer->len,
                                                        expected_parameter->name);
    instr_buffer_ptr arg_buffer = instr_buffer_init(prefix);
    free(prefix);

    rule_argument(stack, table, expected_parameter, arg_buffer);

    argument_count = rule_argument_next(stack, table, function, instr_buffer, argument_count, variadic);

    // append argument instructions
    for (int i = 0; i < arg_buffer->len; i++)
    {
        instr_buffer_append(instr_buffer, arg_buffer->instructions[i]);
    }
    free(arg_buffer);

    return argument_count;
}

// <prog> -> <?php <statement-list> ?>
void rule_prog(stack_ptr stack, sym_table_ptr table, instr_buffer_ptr instr)
{
    DEBUG_RULE();

    // Prolog
    ASSERT_NEXT_TOKEN(stack, TOKEN_OPENING_TAG);
    ASSERT_NEXT_TOKEN(stack, TOKEN_DECLARE);
    // Start parsing the main program body.
    rule_statement_list(stack, table, NULL, instr);

    // Optional closing tag
    token_ptr closing = peek_top(stack);

    if (closing != NULL)
    {
        ASSERT_NEXT_TOKEN(stack, TOKEN_CLOSING_TAG);
    }
}

void parse(stack_ptr stack)
{
    global_table = sym_init();

    sym_table_ptr table = sym_init();

    // Add built in functions to symtable

    function_ptr fn_write = function_create("write", TYPE_VOID, false);
    fn_write->variadic = true;
    fn_write->builtin = true;
    sym_insert_fn(global_table, fn_write);
    BUILT_IN_WRITE(fn_write->instr_buffer);

    function_ptr fn_reads = function_create("reads", TYPE_STRING, true);
    fn_reads->builtin = true;
    sym_insert_fn(global_table, fn_reads);
    BUILT_IN_READS(fn_reads->instr_buffer);

    function_ptr fn_readi = function_create("readi", TYPE_INT, true);
    fn_readi->builtin = true;
    sym_insert_fn(global_table, fn_readi);
    BUILT_IN_READI(fn_readi->instr_buffer);

    function_ptr fn_readf = function_create("readf", TYPE_FLOAT, true);
    fn_readf->builtin = true;
    sym_insert_fn(global_table, fn_readf);
    BUILT_IN_READF(fn_readf->instr_buffer);

    function_ptr fn_chr = function_create("chr", TYPE_STRING, false);
    fn_chr->builtin = true;
    sym_insert_fn(global_table, fn_chr);
    append_parameter(fn_chr, "$i", TYPE_INT, false);
    BUILT_IN_CHR(fn_chr->instr_buffer);

    function_ptr fn_ord = function_create("ord", TYPE_INT, false);
    fn_ord->builtin = true;
    sym_insert_fn(global_table, fn_ord);
    append_parameter(fn_ord, "$c", TYPE_STRING, false);
    BUILT_IN_ORD(fn_ord->instr_buffer);

    function_ptr fn_strlen = function_create("strlen", TYPE_INT, false);
    fn_strlen->builtin = true;
    sym_insert_fn(global_table, fn_strlen);
    append_parameter(fn_strlen, "$s", TYPE_STRING, false);
    BUILT_IN_STRLEN(fn_strlen->instr_buffer);

    function_ptr fn_substring = function_create("substring", TYPE_STRING, true);
    fn_substring->builtin = true;
    sym_insert_fn(global_table, fn_substring);
    append_parameter(fn_substring, "$s", TYPE_STRING, false);
    append_parameter(fn_substring, "$i", TYPE_INT, false);
    append_parameter(fn_substring, "$j", TYPE_INT, false);
    BUILT_IN_SUBSTRING(fn_substring->instr_buffer);

    function_ptr fn_floatval = function_create("floatval", TYPE_FLOAT, false);
    fn_floatval->builtin = true;
    sym_insert_fn(global_table, fn_floatval);
    append_parameter(fn_floatval, "$s", TYPE_ANY, false);
    BUILT_IN_FLOATVAL(fn_floatval->instr_buffer);

    function_ptr fn_intval = function_create("intval", TYPE_INT, false);
    fn_intval->builtin = true;
    sym_insert_fn(global_table, fn_intval);
    append_parameter(fn_intval, "$s", TYPE_ANY, false);
    BUILT_IN_INTVAL(fn_intval->instr_buffer);

    function_ptr fn_strval = function_create("strval", TYPE_STRING, false);
    fn_strval->builtin = true;
    sym_insert_fn(global_table, fn_strval);
    append_parameter(fn_strval, "$s", TYPE_ANY, false);
    BUILT_IN_STRVAL(fn_strval->instr_buffer);

    DEBUG("Running preparser # parse_function_definitions");

    parse_function_definitions(stack);

    DEBUG("Preparsed functions and builtins:");

    if (LOG_ENABLED(PREP))
    {
        sym_print(global_table);

        int count_d = 0;
        function_ptr *functions_d = sym_get_functions(global_table, &count_d);

        for (int i = 0; i < count_d; i++)
        {
            function_ptr function = functions_d[i];

            char *s = function_to_string(function);
            printf("%s\n", s);
        }
    }

    DEBUG("Running parser");

    // Initialize instruction buffer

    instr_buffer_ptr instr_buffer = instr_buffer_init(NULL);

    rule_prog(stack, table, instr_buffer);

    if (stack_size(stack) != 0)
    {
        fprintf(stderr, "SA failed, symbols left on the input stack.\n");
        exit(FAIL_SYNTAX);
    }

    if (LOG_ENABLED(SYMTABLE))
    {
        sym_print(table);
        sym_print(global_table);
    }

    // generate defvars for all the variables in main body at the start of the code
    int var_count = 0;
    variable_ptr *variables = sym_get_variables(table, &var_count);

    for (int i = 0; i < var_count; i++)
    {
        variable_ptr var = variables[i];

        // prepend it before all other instructions
        instr_buffer_prepend(instr_buffer, generate_instruction_ops(INSTR_DEFVAR, 1, instr_var(FRAME_LOCAL, var->name)));
    }

    // prepend file header
    instr_buffer_prepend(instr_buffer, generate_instruction(INSTR_PUSH_FRAME));
    instr_buffer_prepend(instr_buffer, generate_instruction(INSTR_CREATE_FRAME));
    instr_buffer_prepend(instr_buffer, alloc_str(".ifjcode22"));

    // Add exit instr
    INSTRUCTION_OPS(instr_buffer, INSTR_EXIT, 1, alloc_str("int@0"));

    // Output main body
    instr_buffer_out(instr_buffer);
    instr_buffer_dispose(instr_buffer);

    // Output function code

    int count = 0;
    function_ptr *functions = sym_get_functions(global_table, &count);

    for (int i = 0; i < count; i++)
    {
        function_ptr function = functions[i];

        if (function->called == false)
        {
            instr_buffer_dispose(function->instr_buffer);
            continue;
        }

        // only do this for non-built in functions
        if (function->builtin == false)
        {
            // generate defvars for all the variables in main body at the start of the code
            var_count = 0;
            variables = sym_get_variables(function->symtable, &var_count);

            for (int i = 0; i < var_count; i++)
            {
                variable_ptr var = variables[i];

                // prepend it before all other instructions
                instr_buffer_prepend(function->instr_buffer, generate_instruction_ops(INSTR_DEFVAR, 1, instr_var(FRAME_LOCAL, var->name)));
            }

            instr_buffer_prepend(function->instr_buffer, generate_instruction(INSTR_PUSH_FRAME));
            instr_buffer_prepend(function->instr_buffer, generate_instruction(INSTR_CREATE_FRAME));
            instr_buffer_prepend(function->instr_buffer, generate_instruction_ops(INSTR_LABEL, 1, alloc_str(function->name)));
        }

        instr_buffer_out(function->instr_buffer);
        instr_buffer_dispose(function->instr_buffer);
    }

    sym_dispose(table);
    sym_dispose(global_table);
}
