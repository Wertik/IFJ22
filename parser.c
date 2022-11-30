#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "symtable.h"
#include "stack.h"
#include "utils.h"

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
    symbol_ptr symbol = stack_top(stack)->symbol;
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
    item_ptr top = stack_top(stack);

    if (top == NULL || top->symbol == NULL)
    {
        return NULL;
    }

    symbol_ptr symbol = top->symbol;
    token_ptr token = symbol->token;

    stack_pop(stack);

    DEBUG_TOKEN(token);
    DEBUG_STACK_TOP(stack, 2);

    free(symbol);
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
type_t parse_expression(stack_ptr in_stack, table_node_ptr *sym_global)
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
        fprintf(stderr, "Not a valid expression.\n");
        exit(FAIL_SYNTAX);
    }

    return result_type;
}

void rule_expression_next(stack_ptr in_stack, table_node_ptr *sym_global)
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
void rule_statement(stack_ptr in_stack, table_node_ptr *sym_global, function_ptr function)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);
    token_ptr after_next = peek(in_stack, 1);

    // var_id =
    if (next->type == TOKEN_VAR_ID && after_next != NULL && after_next->type == TOKEN_ASSIGN)
    {
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_VAR_ID);

        // <statement> -> var_id = <expression>;
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_ASSIGN);

        int value = parse_expression(in_stack, sym_global);

        // Create symboltable entry if not already present
        if (sym_get_variable(*sym_global, next->value.string) == NULL)
        {
            // TODO: Infer type from value (requires PSA)
            // TODO-CHECK: Nullable by default?
            variable_ptr variable = variable_create(TYPE_INT, true);
            *sym_global = sym_insert(*sym_global, next->value.string, NULL, variable);
        }

        DEBUG_PSEUDOF("%s <- %d", next->value.string, value);

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);
    }
    else if (next->type == TOKEN_ID)
    {
        // <statement> -> id(<argument-list>);

        // TODO: Check that the function exists.
        // TODO: Generate call IFJcode22.
        // TODO: Check parameter count and types

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_ID);

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);

        rule_parameter_list(in_stack, sym_global);

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_R_PAREN);
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);
    }
    else if (next->type == TOKEN_KEYWORD)
    {
        // if / function / while

        ASSERT_NEXT_TOKEN(in_stack, TOKEN_KEYWORD);

        switch (next->value.keyword)
        {
        case KEYWORD_IF:
        {
            // <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}

            // if (<expression>) {<statement-list>}

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);

            type_t type = parse_expression(in_stack, sym_global);

            assert_n_tokens(in_stack, 2, TOKEN_R_PAREN, TOKEN_LC_BRACKET);

            DEBUG_PSEUDOF("if (%s)", type_to_name(type));

            rule_statement_list(in_stack, sym_global, function);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end if");

            // else { <statement-list> }

            ASSERT_KEYWORD(in_stack, KEYWORD_ELSE);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_LC_BRACKET);

            DEBUG_PSEUDO("else");

            rule_statement_list(in_stack, sym_global, function);

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

            DEBUG_PSEUDOF("while (%s)", type_to_name(type));

            rule_statement_list(in_stack, sym_global, function);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDO("end while");
            break;
        }
        case KEYWORD_FUNCTION:
        {
            // <statement> -> function id(<argument-list>) {<statement-list>}

            // Save function to symtable
            token_ptr function_id = assert_next_token_get(in_stack, TOKEN_ID);

            function_ptr function = function_create();
            *sym_global = sym_insert(*sym_global, function_id->value.string, function, NULL);

            DEBUG_PSEUDOF("function %s(...)", function_id->value.string);

            // Parse arguments

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);

            rule_argument_list(in_stack, sym_global, function);

            for (int i = 0; i < function->parameter_count; i++)
            {
                DEBUG_PSEUDOF("Parameter %d: %s %s", i, type_to_name(function->parameters[i].type), function->parameters[i].name);
            }

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_R_PAREN);

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_COLON);

            // Save return type to symtable

            token_ptr return_type = assert_next_token_get(in_stack, TOKEN_TYPE);

            function->return_type = return_type->value.type;

            DEBUG_PSEUDOF("Returns %s", type_to_name(function->return_type));

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_LC_BRACKET);

            // Function statement list

            rule_statement_list(in_stack, sym_global, function);

            if (function->return_type != TYPE_VOID && function->has_return == false)
            {
                fprintf(stderr, "Missing return statement for non-void function %s.", function_id->value.string);
                exit(FAIL_LEXICAL); // TODO: Correct code
            }

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_RC_BRACKET);

            DEBUG_PSEUDOF("end function %s", function_id->value.string);

            token_dispose(function_id);
            break;
        }
        case KEYWORD_RETURN:
        {
            if (function->return_type == TYPE_VOID)
            {
                DEBUG_PSEUDO("return;");
            }
            else
            {
                type_t result_type = parse_expression(in_stack, sym_global);

                if (function->return_type != result_type)
                {
                    fprintf(stderr, "Invalid function return type.\n");
                    exit(FAIL_SEMANTIC);
                }

                DEBUG_PSEUDOF("return %s;", type_to_name(result_type));
            }

            function->has_return = true;

            ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);
            break;
        }
        default:
            fprintf(stderr, "Invalid keyword in statement.\n");
            exit(FAIL_LEXICAL); // TODO: Correct code
        }
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
void rule_statement_list(stack_ptr in_stack, table_node_ptr *sym_global, function_ptr function)
{
    DEBUG_RULE();

    // Decide based on first? There doesn't have to be a statement...

    token_ptr next = peek_top(in_stack);

    if (next == NULL)
    {
        return;
    }

    if ((next->type == TOKEN_KEYWORD) ||
        next->type == TOKEN_VAR_ID ||
        next->type == TOKEN_ID ||
        next->type == TOKEN_CONST_INT ||
        next->type == TOKEN_CONST_DOUBLE ||
        next->type == TOKEN_STRING_LIT)
    {
        // <statement-list> -> <statement>;<statement-list>
        rule_statement(in_stack, sym_global, function);

        rule_statement_list(in_stack, sym_global, function);
    }
    else
    {
        // <statement-list> -> eps
        return;
    }
}

// <arg-list> -> eps
// <arg-list> -> type var_id <arg-next>
void rule_argument_list(stack_ptr in_stack, table_node_ptr *sym_global, function_ptr function)
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
            exit(FAIL_LEXICAL); // TODO: Correct code
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
void rule_argument_next(stack_ptr in_stack, table_node_ptr *sym_global, function_ptr function)
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
void rule_parameter_list(stack_ptr in_stack, table_node_ptr *sym_global)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);
    switch (next->type)
    {
    case TOKEN_VAR_ID:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_VAR_ID);
        rule_parameter_next(in_stack, sym_global);
        break;
        // technicky oba case rovnake asi by bolo lepsie dat or ale neviem
    // predpokladam ze nedostanem do funkcie nejaky vzorec ale len cislo alebo string
    // nedoriesene v scannery pre float pravdepodobne

    // assuming constatnt expression is for all types
    case TOKEN_CONST_INT:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_CONST_INT);
        rule_parameter_next(in_stack, sym_global);
        break;
    case TOKEN_CONST_DOUBLE:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_CONST_DOUBLE);
        rule_parameter_next(in_stack, sym_global);
        break;
    case TOKEN_STRING_LIT:
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_STRING_LIT);
        rule_parameter_next(in_stack, sym_global);
        break;
    default:
        break;
    }
}

// <par-next> -> eps
// <par-next> -> , <par-list>
void rule_parameter_next(stack_ptr in_stack, table_node_ptr *sym_global)
{
    DEBUG_RULE();

    token_ptr next = peek_top(in_stack);

    if (next->type == TOKEN_COMMA)
    {
        // <par-next> -> , <par-list>
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_COMMA);
        rule_parameter_list(in_stack, sym_global);
    }
    else
    {
        // <par-next> -> eps
        return;
    }
}

// <prog> -> <?php <statement> ?>
void rule_prog(stack_ptr in_stack, table_node_ptr *sym_global)
{
    DEBUG_RULE();

    // Parse prolog

    ASSERT_NEXT_TOKEN(in_stack, TOKEN_OPENING_TAG);

    ASSERT_ID(in_stack, "php");
    ASSERT_ID(in_stack, "declare");

    ASSERT_NEXT_TOKEN(in_stack, TOKEN_L_PAREN);

    ASSERT_ID(in_stack, "strict_types");
    ASSERT_NEXT_TOKEN(in_stack, TOKEN_ASSIGN);

    token_ptr const_int = assert_next_token_get(in_stack, TOKEN_CONST_INT);

    if (const_int->value.integer != 1)
    {
        fprintf(stderr, "Strict types has to be enabled.\n");
        exit(FAIL_LEXICAL); // TODO: Correct code
    }

    token_dispose(const_int);

    ASSERT_NEXT_TOKEN(in_stack, TOKEN_R_PAREN);

    ASSERT_NEXT_TOKEN(in_stack, TOKEN_SEMICOLON);

    // Start parsing the main program body.

    rule_statement_list(in_stack, sym_global, NULL);

    token_ptr closing = peek_top(in_stack);

    // Closing tag optional

    if (closing != NULL)
    {
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_CLOSING_TAG);
    }
}

void rule_prog_end(stack_ptr in_stack, table_node_ptr *sym_global)
{
    token_ptr next = peek_top(in_stack);
    if (next->type == TOKEN_NULLABLE)
    {
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_NULLABLE);
        ASSERT_NEXT_TOKEN(in_stack, TOKEN_MORE);
    }
}

void parse(stack_ptr stack)
{
    table_node_ptr sym_global = sym_init();

    rule_prog(stack, &sym_global);

    if (stack_size(stack) != 0)
    {
        fprintf(stderr, "SA failed, symbols left on the input stack.\n");
        exit(FAIL_LEXICAL); // TODO: Correct code
    }
}
