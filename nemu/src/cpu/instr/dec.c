#include "cpu/instr.h"
/*
Put the implementations of `dec' instructions here.
*/

static void instr_execute_1op()
{
    operand_read(&opr_src);
    
    uint8_t CF = cpu.eflags.CF;
    opr_src.val = alu_sub(1, opr_src.val, data_size);
    
    operand_write(&opr_src);
    cpu.eflags.CF = CF;
}

make_instr_impl_1op(dec, r, v)
make_instr_impl_1op(dec, rm, v)