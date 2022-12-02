#include "instruction.h"

int main()
{
    instr_buffer_ptr instr_buffer = instr_buffer_init();

    INSTRUCTION(instr_buffer, INSTR_CREATE_FRAME);
    INSTRUCTION(instr_buffer, INSTR_PUSH_FRAME);

    INSTRUCTION_OPS(instr_buffer, INSTR_DEFVAR, 1, "LF@$x");
    INSTRUCTION_OPS(instr_buffer, INSTR_MOVE, 2, "LF@$x", "int@10");

    INSTRUCTION_OPS(instr_buffer, INSTR_DPRINT, 1, "LF@$x");

    INSTRUCTION_OPS(instr_buffer, INSTR_EXIT, 1, "int@0");

    instr_buffer_out(instr_buffer);

    instr_buffer_dispose(instr_buffer);
    return 0;
}
