#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "symtable.h"
#include "stack.h"
#include "utils.h"
#include "instruction.h"

int different_parse = 0;

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

// treba asi zmenit
type_t parse_expression(stack_ptr in_stack, sym_table_ptr sym_global)
{
    DEBUG_RULE();

    type_t result_type;
    token_ptr next = peek_top(in_stack);

    switch (next->type)
    {
    case TOKEN_L_PAREN:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);
        result_type = parse_expression(in_stack, sym_global);
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_R_PAREN);
        break;

    case TOKEN_CONST_DOUBLE:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_CONST_DOUBLE);

        result_type = TYPE_FLOAT;
        rule_expression_next(in_stack, sym_global);
        break;
    case TOKEN_CONST_INT:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_CONST_INT);
        result_type = TYPE_INT;
        rule_expression_next(in_stack, sym_global);
        break;
    case TOKEN_STRING_LIT:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_STRING_LIT);
        // value = parse_expression(in_stack);

        // how to return string
        // value = next->value.string;

        result_type = TYPE_STRING;

        rule_expression_next(in_stack, sym_global);
        break;
    case TOKEN_VAR_ID:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_VAR_ID);

        // TODO: Check if var exists in symtable, get the type.

        // WILL HAVE TO WORK WITH SYMBOL TABLE
        // value = parse_expression(in_stack);
        // value = next->value.string;
        rule_expression_next(in_stack, sym_global);
        break;
    // Part of FUNEXP
    /* case TOKEN_ID:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_ID);

        // TODO: Check if function exists and get the return type from symtable.

        // value = parse_expression(in_stack);
        // value = next->value.string;
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);
        rule_argument_list(in_stack, sym_global);
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_R_PAREN);
        break; */
    default:
        fprintf(stderr, "Not an expression.\n");
        exit(FAIL_SYNTAX);
    }

    return result_type;
}

void rule_expression_next(stack_ptr in_stack, sym_table_ptr sym_global)
{
    token_ptr next = peek_top(in_stack);
    // is it actually necessary?
    // int value;
    switch (next->type)
    {
    case TOKEN_PLUS:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_PLUS);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_MINUS:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_MINUS);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_MULTIPLE:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_MULTIPLE);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_DIVIDE:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_DIVIDE);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_DOT:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_DOT);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_EQUAL:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_EQUAL);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_NOT_EQUAL:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_NOT_EQUAL);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_MORE:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_MORE);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_MORE_EQUAL:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_MORE_EQUAL);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_LESS:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_LESS);
        parse_expression(in_stack, sym_global);
        break;
    case TOKEN_LESS_EQUAL:

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_LESS_EQUAL);
        parse_expression(in_stack, sym_global);
        break;
    default:
    {
    }
        // $x = <10>; -> 10 is a valid expression
        /* default:
            fprintf(stderr, "non valid expression\n");
            exit(FAIL_LEXICAL); */
    }
}

// <statement> -> var_id = <expression>;
// <statement> -> return <expression>;
// <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}
// <statement> -> while (<expression>) {<statement-list>}
// <statement> -> function id(<argument-list>) {<statement-list>}
// <statement> -> id(<argument-list>);
// <statement> -> <expression>;
void rule_statement(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function, instr_buffer_ptr instr_buffer)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);
    token_ptr after_next = peek(in_stack, 1);

    // <statement> -> var_id = <expression>;
    if (next->type == TOKEN_VAR_ID && after_next != NULL && after_next->type == TOKEN_ASSIGN)
    {
        next = get_next_token(in_stack);
        // already know the next token, just pop it from the stack
        stack_pop(in_stack);

        // parse r-side
        type_t result_type = parse_expression(in_stack, sym_global);

        // assign to l-side
        variable_ptr var = sym_get_variable(sym_global, next->value.string);

        // Create symboltable entry if not already present
        if (var == NULL)
        {
            // TODO: Infer type from value (requires PSA)
            variable_ptr variable = variable_create(TYPE_INT, true);
            sym_insert(sym_global, next->value.string, NULL, variable);

            INSTRUCTION_OPS(instr_buffer, INSTR_DEFVAR, 1, "LF@$x");
        }
        else
        {
            // The entry exists, change the type
            var->type = result_type;
        }

        INSTRUCTION_OPS(instr_buffer, INSTR_MOVE, 2, instr_var(FRAME_LOCAL, next->value.string), "int@10");
        INSTRUCTION_OPS(instr_buffer, INSTR_DPRINT, 1, instr_var(FRAME_LOCAL, next->value.string));

        DEBUG_PSEUDO("%s <- %s", next->value.string, type_to_name(result_type));

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);
        token_dispose(next);
    }
    else if (next->type == TOKEN_ID)
    {
        // <statement> -> id(<argument-list>);

        // TODO: Generate call IFJcode22.
        // TODO: Check parameter count and types

        // Check that the function exists.
        token_ptr func_id = assert_next_token_get(in_stack, TOKEN_ID);

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);

        function_ptr function = sym_get_function(sym_global, func_id->value.string);

        if (function == NULL)
        {
            fprintf(stderr, "Function %s not defined.\n", func_id->value.string);
            exit(FAIL_SEMANTIC_FUNC_DEF);
        }

        rule_parameter_list(in_stack, sym_global, function, 0, function->variadic);

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_R_PAREN);
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);
        token_dispose(func_id);
    }
    else if (next->type == TOKEN_KEYWORD)
    {
        // if / function / while

        next = assert_next_token_get(in_stack, TOKEN_KEYWORD);

        switch (next->value.keyword)
        {
        case KEYWORD_IF:
        {
            // <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}

            // if (<expression>) {<statement-list>}

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);

            type_t type = parse_expression(in_stack, sym_global);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_PSEUDO("if (%s)", type_to_name(type));

            rule_statement_list(in_stack, sym_global, function, instr_buffer);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end if");

            // else { <statement-list> }

            ASSERT_KEYWORD(in_stack, KEYWORD_ELSE);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_LC_BRACKET);

            DEBUG_PSEUDO("else");

            rule_statement_list(in_stack, sym_global, function, instr_buffer);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end else");
            break;
        }
        case KEYWORD_WHILE:
        {
            // <statement> -> while (<expression>) {<statement-list>}
            ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);

            type_t type = parse_expression(in_stack, sym_global);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_PSEUDO("while (%s)", type_to_name(type));

            rule_statement_list(in_stack, sym_global, function, instr_buffer);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end while");
            break;
        }
        case KEYWORD_FUNCTION:
        {
            // <statement> -> function id(<argument-list>) : type {<statement-list>}

            // Save function to symtable
            token_ptr function_id = assert_next_token_get(in_stack, TOKEN_ID);

            // Check if the function is already defined

            if (sym_search(sym_global, function_id->value.string) != NULL)
            {
                fprintf(stderr, "Function %s already defined.\n", function_id->value.string);
                exit(FAIL_SEMANTIC_FUNC_DEF);
            }

            function_ptr function = function_create();
            sym_insert(sym_global, function_id->value.string, function, NULL);

            DEBUG_PSEUDO("function %s(...)", function_id->value.string);

            // Parse arguments

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);

            rule_argument_list(in_stack, sym_global, function);

            for (int i = 0; i < function->parameter_count; i++)
            {
                DEBUG_PSEUDO("Parameter %d: %s %s", i, type_to_name(function->parameters[i].type), function->parameters[i].name);

                // Insert as variables into function local symtable
                variable_ptr parameter = variable_create(function->parameters[i].type, function->parameters[i].type_nullable);
                sym_insert(function->symtable, function->parameters[i].name, NULL, parameter);
            }

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_R_PAREN);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_COLON);

            // Save return type to symtable

            token_ptr return_type = assert_next_token_get(in_stack, TOKEN_TYPE);

            function->return_type = return_type->value.type;

            DEBUG_PSEUDO("Returns %s", type_to_name(function->return_type));

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_LC_BRACKET);

            // Function statement list

            rule_statement_list(in_stack, sym_global, function, instr_buffer);

            if (function->return_type != TYPE_VOID && function->has_return == false)
            {
                fprintf(stderr, "Missing return statement for non-void function %s.\n", function_id->value.string);
                exit(FAIL_SEMANTIC_INVALID_RETURN_COUNT);
            }

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end function %s", function_id->value.string);

            token_dispose(function_id);
            token_dispose(return_type);
            break;
        }
        case KEYWORD_RETURN:
        {
            // Global return
            if (function == NULL)
            {
                // Can return a value or not.
                token_ptr next = peek_top(in_stack);

                type_t result_type = TYPE_VOID;

                // not a semicolon, has to be an expression or constant
                if (next->type != TOKEN_SEMICOLON)
                {
                    result_type = parse_expression(in_stack, sym_global);
                }

                DEBUG_PSEUDO("global return %s;", type_to_name(result_type));
                ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);
            }
            else
            {
                // Function return

                if (function->return_type == TYPE_VOID)
                {
                    DEBUG_PSEUDO("return;");
                }
                else
                {
                    type_t result_type = parse_expression(in_stack, sym_global);

                    if (function->return_type != result_type)
                    {
                        fprintf(stderr, "Invalid function return type. Expected %s, got %s.\n", type_to_name(function->return_type), type_to_name(result_type));
                        exit(FAIL_SEMANTIC_INVALID_RETURN_TYPE);
                    }

                    DEBUG_PSEUDO("return %s;", type_to_name(result_type));
                }
                function->has_return = true;
                ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);
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
        parse_expression(in_stack, sym_global);

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);
    }
}

// <statement-list> -> <statement><statement-list>
// <statement-list> -> eps
void rule_statement_list(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function, instr_buffer_ptr instr_buffer)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    // Nothing else to parse
    if (next == NULL)
    {
        // <statement-list> -> eps
        return;
    }

    if ((next->type == TOKEN_KEYWORD) ||
        next->type == TOKEN_VAR_ID ||
        next->type == TOKEN_ID ||
        next->type == TOKEN_CONST_INT ||
        next->type == TOKEN_CONST_DOUBLE ||
        next->type == TOKEN_STRING_LIT)
    {
        // <statement-list> -> <statement><statement-list>
        rule_statement(in_stack, sym_global, function, instr_buffer);

        rule_statement_list(in_stack, sym_global, function, instr_buffer);
    }
    else
    {
        // <statement-list> -> eps
        return;
    }
}

// <arg-list> -> eps
// <arg-list> -> type var_id <arg-next>
void rule_argument_list(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    if (next->type == TOKEN_TYPE)
    {
        // <arg-list> -> type var_id <arg-next>

        token_ptr arg_type = assert_next_token_get(in_stack, TOKEN_TYPE);
        token_ptr arg_id = assert_next_token_get(in_stack, TOKEN_VAR_ID);

        // cannot use void as parameter type
        if (arg_type->value.type == TYPE_VOID)
        {
            fprintf(stderr, "VOID is not a valid type of arguments.\n");
            exit(FAIL_SYNTAX);
        }

        // Append parameter
        // TODO: Nullable?
        append_parameter(function, arg_id->value.string, arg_type->value.type, false);

        token_dispose(arg_type);
        token_dispose(arg_id);

        rule_argument_next(in_stack, sym_global, function);
    }
    else
    {
        // <arg-list> -> eps
        return;
    }
}

// <arg-next> -> eps
// <arg-next> -> , <arg-list>
void rule_argument_next(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    if (next->type == TOKEN_COMMA)
    {
        // <arg-next> -> , <arg-list>
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_COMMA);

        rule_argument_list(in_stack, sym_global, function);
    }
    else
    {
        // <arg-next> -> eps
        return;
    }
}

// <par-list> -> eps
// <par-list> -> var_id <par-next>
// <par-list> -> const_int <par-next>
// <par-list> -> const_float <par-next>
// <par-list> -> string_list <par-next>
void rule_parameter_list(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function, int current_parameter, bool variadic)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    // Ignore argument count checks with variadic functions

    // <par-list> -> eps
    if (!is_one_of(next, 4, TOKEN_VAR_ID, TOKEN_CONST_INT, TOKEN_CONST_DOUBLE, TOKEN_STRING_LIT))
    {
        if (!variadic && current_parameter < function->parameter_count - 1)
        {
            fprintf(stderr, "Not enough parameters for function. Expected %d but got %d.\n", function->parameter_count, current_parameter);
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }
        return;
    }

    if (!variadic && current_parameter + 1 > function->parameter_count)
    {
        fprintf(stderr, "Too many parameters for function. Expected %d but got %d.\n", function->parameter_count, current_parameter + 1);
        exit(FAIL_SEMANTIC_BAD_ARGS);
    }

    parameter_t expected_parameter = function->parameters[current_parameter];

    switch (next->type)
    {
    case TOKEN_VAR_ID:
    {
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_VAR_ID);
        rule_parameter_next(in_stack, sym_global, function, current_parameter, variadic);
        break;
    }
    case TOKEN_CONST_INT:
    {
        if (expected_parameter.type != TYPE_INT && variadic != true)
        {
            fprintf(stderr, "Bad parameter type for %s. Expected %s but got %s.\n", expected_parameter.name, type_to_name(expected_parameter.type), "TYPE_INT");
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_CONST_INT);
        rule_parameter_next(in_stack, sym_global, function, current_parameter, variadic);
        break;
    }
    case TOKEN_CONST_DOUBLE:
    {
        if (expected_parameter.type != TYPE_FLOAT && variadic != true)
        {
            fprintf(stderr, "Bad parameter type for %s. Expected %s but got %s.\n", expected_parameter.name, type_to_name(expected_parameter.type), "TYPE_FLOAT");
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_CONST_DOUBLE);
        rule_parameter_next(in_stack, sym_global, function, current_parameter, variadic);
        break;
    }
    case TOKEN_STRING_LIT:
    {

        if (expected_parameter.type != TYPE_STRING && variadic != true)
        {
            fprintf(stderr, "Bad parameter type for %s. Expected %s but got %s.\n", expected_parameter.name, type_to_name(expected_parameter.type), "TYPE_STRING");
            exit(FAIL_SEMANTIC_BAD_ARGS);
        }

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_STRING_LIT);
        rule_parameter_next(in_stack, sym_global, function, current_parameter, variadic);
        break;
    }
    default:
        fprintf(stderr, "Invalid token in function parameter.\n");
        exit(FAIL_SYNTAX);
    }
}

// <par-next> -> eps
// <par-next> -> , <par-list>
void rule_parameter_next(stack_ptr in_stack, sym_table_ptr sym_global, function_ptr function, int current_parameter, bool variadic)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    if (next->type == TOKEN_COMMA)
    {
        // <par-next> -> , <par-list>
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_COMMA);
        rule_parameter_list(in_stack, sym_global, function, current_parameter + 1, variadic);
    }
    else
    {
        // <par-next> -> eps
        return;
    }
}

// <prog> -> <?php <statement-list> ?>
void rule_prog(stack_ptr in_stack, sym_table_ptr sym_global, instr_buffer_ptr instr_buffer)
{
    DEBUG_RULE();

    // Main frame body
    INSTRUCTION(instr_buffer, INSTR_CREATE_FRAME);
    INSTRUCTION(instr_buffer, INSTR_PUSH_FRAME);

    // Prolog
    ASSERT_NEXT_TOKEN(in_stack, TOKEN_OPENING_TAG);
    ASSERT_NEXT_TOKEN(in_stack, TOKEN_DECLARE);

    // Start parsing the main program body.
    rule_statement_list(in_stack, sym_global, NULL, instr_buffer);

    // Optional closing tag
    token_ptr closing = peek_top(in_stack);

    if (closing != NULL)
    {
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_CLOSING_TAG);
    }
}

void parse(stack_ptr stack)
{
    sym_table_ptr sym_global = sym_init();

    // Add built in functions to symtable

    function_ptr fn_write = function_create();
    fn_write->variadic = true;
    sym_insert(sym_global, "write", fn_write, NULL);

    function_ptr fn_reads = function_create();
    fn_reads->return_type = TYPE_STRING;
    fn_reads->return_type_nullable = true;
    sym_insert(sym_global, "reads", fn_reads, NULL);

    function_ptr fn_readi = function_create();
    fn_readi->return_type = TYPE_INT;
    fn_readi->return_type_nullable = true;
    sym_insert(sym_global, "readi", fn_readi, NULL);

    function_ptr fn_readf = function_create();
    fn_readf->return_type = TYPE_FLOAT;
    fn_readf->return_type_nullable = true;
    sym_insert(sym_global, "readf", fn_readf, NULL);

    // Initialize instruction buffer

    instr_buffer_ptr instr_buffer = instr_buffer_init();

    rule_prog(stack, sym_global, instr_buffer);

    if (stack_size(stack) != 0)
    {
        fprintf(stderr, "SA failed, symbols left on the input stack.\n");
        exit(FAIL_SYNTAX);
    }

    // Add exit instr
    INSTRUCTION_OPS(instr_buffer, INSTR_EXIT, 1, "int@0");

    instr_buffer_out(instr_buffer);
    instr_buffer_dispose(instr_buffer);

    sym_dispose(sym_global);
}
