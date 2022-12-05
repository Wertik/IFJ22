/*
 * Project: IFJ22 language compiler
 *
 * @author xotrad00 Martin Otradovec
 */

#include "instruction.h"
#include "utils.h"
#include "scanner.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *instruction_to_str(instruction_t instruction)
{
    switch (instruction)
    {
    case INSTR_MOVE:
        return "MOVE";
    case INSTR_CREATE_FRAME:
        return "CREATEFRAME";
    case INSTR_POP_FRAME:
        return "POPFRAME";
    case INSTR_PUSH_FRAME:
        return "PUSHFRAME";
    case INSTR_DEFVAR:
        return "DEFVAR";
    case INSTR_CALL:
        return "CALL";
    case INSTR_RETURN:
        return "RETURN";
    case INSTR_PUSHS:
        return "PUSHS";
    case INSTR_POPS:
        return "POPS";
    case INSTR_CLEARS:
        return "CLEARS";
    case INSTR_ADD:
        return "ADD";
    case INSTR_SUB:
        return "SUB";
    case INSTR_MUL:
        return "MUL";
    case INSTR_DIV:
        return "DIV";
    case INSTR_IDIV:
        return "IDIV";
    case INSTR_ADDS:
        return "ADDS";
    case INSTR_SUBS:
        return "SUBS";
    case INSTR_MULS:
        return "MULS";
    case INSTR_DIVS:
        return "DIVS";
    case INSTR_IDIVS:
        return "IDIVS";
    case INSTR_LT:
        return "LT";
    case INSTR_GT:
        return "GT";
    case INSTR_EQ:
        return "EQ";
    case INSTR_LTS:
        return "LTS";
    case INSTR_GTS:
        return "GTS";
    case INSTR_EQS:
        return "EQS";
    case INSTR_AND:
        return "AND";
    case INSTR_OR:
        return "OR";
    case INSTR_NOT:
        return "NOT";
    case INSTR_ANDS:
        return "ANDS";
    case INSTR_ORS:
        return "ORS";
    case INSTR_NOTS:
        return "NOTS";
    case INSTR_INT2FLOAT:
        return "INT2FLOAT";
    case INSTR_FLOAT2INT:
        return "FLOAT2INT";
    case INSTR_INT2CHAR:
        return "INT2CHAR";
    case INSTR_STRI2INT:
        return "STRI2INT";
    case INSTR_INT2FLOATS:
        return "INT2FLOATS";
    case INSTR_FLOAT2INTS:
        return "FLOAT2INTS";
    case INSTR_INT2CHARS:
        return "INT2CHARS";
    case INSTR_STRI2INTS:
        return "STRI2INTS";
    case INSTR_READ:
        return "READ";
    case INSTR_WRITE:
        return "WRITE";
    case INSTR_CONCAT:
        return "CONCAT";
    case INSTR_STRLEN:
        return "STRLEN";
    case INSTR_SETCHAR:
        return "SETCHAR";
    case INSTR_TYPE:
        return "TYPE";
    case INSTR_LABEL:
        return "LABEL";
    case INSTR_JUMP:
        return "JUMP";
    case INSTR_JUMPIFEQ:
        return "JUMPIFEQ";
    case INSTR_JUMPIFNEQ:
        return "JUMPIFNEQ";
    case INSTR_JUMPIFEQS:
        return "JUMPIFEQS";
    case INSTR_JUMPIFNEQS:
        return "JUMPIFNEQS";
    case INSTR_EXIT:
        return "EXIT";
    case INSTR_BREAK:
        return "BREAK";
    case INSTR_DPRINT:
        return "DPRINT";
    default:
        fprintf(stderr, "Invalid instruction %d.\n", instruction);
        exit(FAIL_INTERNAL);
    }
}

const char *frame_to_formal(frame_t frame)
{
    switch (frame)
    {
    case FRAME_LOCAL:
        return "LF";
    case FRAME_GLOBAL:
        return "GF";
    case FRAME_TEMP:
        return "TF";
    default:
        fprintf(stderr, "Invalid frame %d.\n", frame);
        exit(FAIL_INTERNAL);
    }
}

char *instr_var(frame_t frame, char *var)
{
    const char *frame_formal = frame_to_formal(frame);
    return dyn_str("%s@%s", frame_formal, var);
}

char *instr_const_int(int val)
{
    return dyn_str("int@%d", val);
}

char *instr_const_float(double val)
{
    return dyn_str("float@%a", val);
}

// Replace a character by a series of characters
char *str_rep(char *str, char target, char *replacement)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == target)
        {
            // realloc with new size
            int size = strlen(str) + strlen(replacement);
            char *s = malloc(sizeof(char) * (size + 1));

            MALLOC_CHECK(s);

            memcpy(s, str, i);
            memcpy(s + i, replacement, strlen(replacement));
            memcpy(s + i + strlen(replacement), str + i + 1, strlen(str) - i);
            s[size] = '\0';

            return str_rep(s, target, replacement);
        }
    }
    return str;
}

char *instr_const_str(char *str)
{
    char *s = dyn_str("string@%s", str);

    s = str_rep(s, ' ', "\\032");
    s = str_rep(s, '\r', "\\013");
    s = str_rep(s, '\n', "\\010");

    return s;
}

char *instr_const_bool(bool val)
{
    return dyn_str("bool@%s", val == true ? "true" : "false");
}

char *instr_type_str(type_t type)
{
    const char *type_str = type_to_formal(type);

    size_t len = snprintf(NULL, 0, "string@%s", type_str);
    char *s = malloc(sizeof(char) * (len + 1));

    MALLOC_CHECK(s);

    sprintf(s, "string@%s", type_str);
    return s;
}

char *dyn_str(const char *fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    size_t len = vsnprintf(NULL, 0, fmt, valist);
    char *s = malloc(sizeof(char) * (len + 1));

    MALLOC_CHECK(s);

    va_start(valist, fmt);
    vsprintf(s, fmt, valist);

    va_end(valist);
    return s;
}

char *generate_instruction(instruction_t instruction)
{
    const char *instruction_str = instruction_to_str(instruction);

    // +1 for null byte
    size_t len = (strlen(instruction_str) + 1);
    char *call_str = malloc(sizeof(char) * len);

    MALLOC_CHECK(call_str);

    strcpy(call_str, instruction_str);

    return call_str;
}

char *generate_instruction_ops(instruction_t instruction, int n, ...)
{
    const char *instruction_str = instruction_to_str(instruction);

    va_list val;
    va_start(val, n);

    // +1 for null byte
    size_t len = (strlen(instruction_str) + 1);
    char *call_str = malloc(sizeof(char) * len);

    MALLOC_CHECK(call_str);

    strcpy(call_str, instruction_str);

    for (int i = 0; i < n; i++)
    {
        char *s = va_arg(val, char *);

        // +1 for a space
        len += strlen(s) + 1;

        // +1 for a space
        call_str = realloc(call_str, sizeof(char) * (len + 1));

        MALLOC_CHECK(call_str);

        call_str = strcat(call_str, " ");

        call_str = strcat(call_str, s);

        free(s);
    }

    va_end(val);
    return call_str;
}

instr_buffer_ptr instr_buffer_init(char *prefix)
{
    instr_buffer_ptr instr_buffer = malloc(sizeof(struct instr_buffer_t));

    MALLOC_CHECK(instr_buffer);

    instr_buffer->len = 0;

    instr_buffer->instructions = malloc(0);
    MALLOC_CHECK(instr_buffer->instructions);

    if (prefix != NULL)
    {
        instr_buffer->prefix = malloc(sizeof(char) * (strlen(prefix) + 1));
        MALLOC_CHECK(instr_buffer->prefix);

        strcpy(instr_buffer->prefix, prefix);
    } else {
        instr_buffer->prefix = NULL;
    }

    return instr_buffer;
}

void instr_buffer_append(instr_buffer_ptr instr_buffer, char *instr)
{
    instr_buffer->len += 1;

    instr_buffer->instructions = realloc(instr_buffer->instructions, sizeof(char *) * instr_buffer->len);
    MALLOC_CHECK(instr_buffer->instructions);

    instr_buffer->instructions[instr_buffer->len - 1] = instr;
}

void instr_buffer_print(instr_buffer_ptr instr_buffer)
{
    printf("instr_buffer(%s)(%ld)[", instr_buffer->prefix == NULL ? "no prefix" : instr_buffer->prefix, instr_buffer->len);
    for (int i = 0; i < instr_buffer->len; i++)
    {
        printf("%s", instr_buffer->instructions[i]);

        if (i != instr_buffer->len - 1)
        {
            printf(",");
        }
    }
    printf("]\n");
}

void instr_buffer_out(instr_buffer_ptr instr_buffer)
{
    for (int i = 0; i < instr_buffer->len; i++)
    {
        printf("%s\n", instr_buffer->instructions[i]);
    }
}

void instr_buffer_dispose(instr_buffer_ptr instr_buffer)
{
    for (int i = 0; i < instr_buffer->len; i++)
    {
        free(instr_buffer->instructions[i]);
    }
    if (instr_buffer->prefix != NULL)
    {
        free(instr_buffer->prefix);
    }
    free(instr_buffer->instructions);
    free(instr_buffer);
}
