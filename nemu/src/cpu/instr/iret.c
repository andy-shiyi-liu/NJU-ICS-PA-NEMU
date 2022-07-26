#include "cpu/instr.h"
/*
Put the implementations of `iret' instructions here.
*/

static inline uint32_t pop()
{
    opr_dest.type = OPR_MEM;
    opr_dest.data_size = data_size;
    opr_dest.addr = cpu.esp;
    operand_read(&opr_dest);
    cpu.esp += 4;

    return opr_dest.val;
}

make_instr_func(iret)
{
    data_size = 32;

    cpu.eip = pop();
    cpu.cs.val = pop() & 0xffff;
    cpu.eflags.val = pop();
    load_sreg(1);

    return 0;
}