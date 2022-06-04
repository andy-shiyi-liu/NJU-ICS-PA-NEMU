#include "cpu/instr.h"
/*
Put the implementations of `or' instructions here.
*/

static void instr_execute_2op()
{
    operand_read(&opr_src);
	operand_read(&opr_dest);

    if(opr_src.data_size == 8)
    {
        opr_src.val = sign_ext(opr_src.val, 8);
    }

    if(opr_dest.data_size == 8)
    {
        opr_dest.val = sign_ext(opr_dest.val, 8);
    }
    
    opr_dest.val = alu_or(opr_src.val, opr_dest.val, data_size);
    
    operand_write(&opr_dest);
    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
}


make_instr_impl_2op(or, r, rm, v)
make_instr_impl_2op(or, r, rm, b)
make_instr_impl_2op(or, rm, r, b)
make_instr_impl_2op(or, i, a, v)
make_instr_impl_2op(or, i, a, b)
make_instr_impl_2op(or, i, rm, bv)