#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/

static void instr_execute_1op()
{
    opr_dest.type = OPR_MEM;
    opr_dest.data_size = data_size;
    opr_dest.addr = cpu.esp;
    operand_read(&opr_dest);
    cpu.esp += 4;
    
    opr_src.val = opr_dest.val;
    opr_src.sreg = SREG_SS;
    
    operand_write(&opr_src);
}

make_instr_impl_1op(pop, r, v)

static inline uint32_t pop()
{
    opr_dest.type = OPR_MEM;
    opr_dest.data_size = data_size;
    opr_dest.addr = cpu.esp;
    operand_read(&opr_dest);
    cpu.esp += 4;

    return opr_dest.val;
}

make_instr_func(popa)
{
    cpu.edi = pop();
    cpu.esi = pop();
    cpu.ebp = pop();
    pop();
    cpu.ebx = pop();
    cpu.edx = pop();
    cpu.ecx = pop();
    cpu.eax = pop();

    return 1;
}
