#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include <string.h>

#define INSTRUCTION(instr_buffer, instr)                                \
    do                                                                  \
    {                                                                   \
        instr_buffer_append(instr_buffer, generate_instruction(instr)); \
    } while (0);

#define INSTRUCTION_OPS(instr_buffer, instr, count, ...)                  \
    do                                                                    \
    {                                                                     \
        char *call = generate_instruction_ops(instr, count, __VA_ARGS__); \
        instr_buffer_append(instr_buffer, call);                          \
    } while (0);

#define TERM(frame, var) (#frame "@" #var)

typedef enum
{
    // var symb
    INSTR_MOVE,

    // Frames

    INSTR_CREATE_FRAME,
    INSTR_POP_FRAME,
    INSTR_PUSH_FRAME,
    // var
    INSTR_DEFVAR,

    // Function calls

    // call
    INSTR_CALL,
    INSTR_RETURN,

    // Stack

    // symb
    INSTR_PUSHS,
    // var
    INSTR_POPS,
    INSTR_CLEARS,

    // Arithmetic

    // var symb symb
    INSTR_ADD,
    // var symb symb
    INSTR_SUB,
    // var symb symb
    INSTR_MUL,
    // var symb symb
    INSTR_DIV,
    // var symb symb
    INSTR_IDIV,

    INSTR_ADDS,
    INSTR_SUBS,
    INSTR_MULS,
    INSTR_DIVS,
    INSTR_IDIVS,

    // Comps

    // var symb symb
    INSTR_LT,
    // var symb symb
    INSTR_GT,
    // var symb symb
    INSTR_EQ,

    INSTR_LTS,
    INSTR_GTS,
    INSTR_EQS,

    // Logic

    // var symb symb
    INSTR_AND,
    // var symb symb
    INSTR_OR,
    // var symb symb
    INSTR_NOT,

    INSTR_ANDS,
    INSTR_ORS,
    INSTR_NOTS,

    // Conversions

    // var symb
    INSTR_INT2FLOAT,
    // var symb
    INSTR_FLOAT2INT,
    // var symb
    INSTR_INT2CHAR,
    // var symb symb
    INSTR_STRI2INT,

    INSTR_INT2FLOATS,
    INSTR_FLOAT2INTS,
    INSTR_INT2CHARS,
    INSTR_STRI2INTS,

    // I/O

    // var type
    INSTR_READ,
    // symb
    INSTR_WRITE,

    // Str

    // var symb symb
    INSTR_CONCAT,
    // var symb
    INSTR_STRLEN,
    // var symb symb
    INSTR_SETCHAR,

    // Type

    // var symb
    INSTR_TYPE,

    // Flow control

    // label
    INSTR_LABEL,
    // label
    INSTR_JUMP,
    // label symb symb
    INSTR_JUMPIFEQ,
    // label symb symb
    INSTR_JUMPIFNEQ,

    INSTR_JUMPIFEQS,
    INSTR_JUMPIFNEQS,

    // symb
    INSTR_EXIT,

    // Debug

    INSTR_BREAK,
    // symb
    INSTR_DPRINT,
} instruction_t;

typedef enum
{
    FRAME_LOCAL,
    FRAME_GLOBAL,
    FRAME_TEMP,
} frame_t;

typedef struct instr_buffer_t
{
    char **instructions;
    size_t len;
} * instr_buffer_ptr;

// Convert enum instruction to instruction keyword
const char *instruction_to_str(instruction_t instr);

char *generate_instruction(instruction_t instr);

// Generate an instruction call with n operands.
char *generate_instruction_ops(instruction_t instr, int n, ...);

// Instruction buffer

instr_buffer_ptr instr_buffer_init();
void instr_buffer_append(instr_buffer_ptr instr_buffer, char *instr);
void instr_buffer_print(instr_buffer_ptr instr_buffer);
void instr_buffer_dispose(instr_buffer_ptr instr_buffer);

// Write the buffer to stdout
void instr_buffer_out(instr_buffer_ptr instr_buffer);

char *instr_var(frame_t frame, char *name);

#endif
