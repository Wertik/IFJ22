#include "instruction.h"
#include "utils.h"
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
        return "#invalid-instruction";
    }
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

// Generate an instruction call with n operands
// <INSTR> <OP1> <OP2> <OP3>
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
    }

    va_end(val);
    return call_str;
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

    size_t len = snprintf(NULL, 0, "%s@%s", frame_formal, var);

    char *s = malloc(sizeof(char) * (len + 1));

    MALLOC_CHECK(s);

    sprintf(s, "%s@%s", frame_formal, var);

    return s;
}

instr_buffer_ptr instr_buffer_init()
{
    instr_buffer_ptr instr_buffer = malloc(sizeof(struct instr_buffer_t));

    MALLOC_CHECK(instr_buffer);

    instr_buffer->len = 0;
    instr_buffer->instructions = malloc(0);

    MALLOC_CHECK(instr_buffer->instructions);

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
    printf("instr_buffer(%ld)[", instr_buffer->len);
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
    printf(".ifjcode22\n"); // Header
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
    free(instr_buffer->instructions);
    free(instr_buffer);
}
