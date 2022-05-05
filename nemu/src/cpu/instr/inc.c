#include "cpu/instr.h"
/*
Put the implementations of `inc' instructions here.
*/

static void instr_execute_1op()
{
    operand_read(&opr_src);
    
    uint8_t CF = cpu.eflags.CF;
    opr_src.val = alu_add(opr_src.val, 1, data_size);
    
    operand_write(&opr_src);
    cpu.eflags.CF = CF;
}

make_instr_impl_1op(inc, rm, v)

make_instr_impl_1op(inc, r, v)