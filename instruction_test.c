/*
 * Project: IFJ22 language compiler
 * 
 * Description:
 * Usage demonstration for instruction generator functions and macros.
 *
 * @author xotrad00 Martin Otradovec
 */

#include "instruction.h"
#include "buffer.h"

int main()
{
    instr_buffer_ptr instr_buffer = instr_buffer_init();

    instr_buffer_append(instr_buffer, alloc_str(".ifjcode22"));

    INSTRUCTION(instr_buffer, INSTR_CREATE_FRAME);
    INSTRUCTION(instr_buffer, INSTR_PUSH_FRAME);

    INSTRUCTION_OPS(instr_buffer, INSTR_DEFVAR, 1, instr_var(FRAME_LOCAL, "$x"));
    INSTRUCTION_OPS(instr_buffer, INSTR_MOVE, 2, instr_var(FRAME_LOCAL, "$x"), instr_const_int(10));

    INSTRUCTION_OPS(instr_buffer, INSTR_DPRINT, 1, instr_var(FRAME_LOCAL, "$x"));

    INSTRUCTION_OPS(instr_buffer, INSTR_EXIT, 1, instr_const_int(0));

    instr_buffer_out(instr_buffer);

    instr_buffer_dispose(instr_buffer);
    return 0;
}
