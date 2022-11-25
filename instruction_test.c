#include "instruction.h"

int main()
{
    instr_buffer_ptr instr_buffer = instr_buffer_init();

    ADD_INSTRUCTION(instr_buffer, INSTR_DEFVAR, 1, "LF@$b");
    ADD_INSTRUCTION(instr_buffer, INSTR_ADD, 3, "LF@$a", "%b", "10.0");
    ADD_INSTRUCTION(instr_buffer, INSTR_CALL, 1, "GF@_hello");

    instr_buffer_print(instr_buffer);

    instr_buffer_dispose(instr_buffer);
    return 0;
}
