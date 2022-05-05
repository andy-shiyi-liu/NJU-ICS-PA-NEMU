#include "cpu/instr.h"
/*
Put the implementations of `lea' instructions here.
*/

/*
make_instr_func(lea);
{
    opr_src.type = OPR_MEM;
    opr_src.data_size = data_size;
    opr_src.addr = cpu.esp;
    opr_src.val = opr_src.val;
    
    opr_dest.type = OPR_MEM;
    opr_dest.data_size = data_size;
    opr_dest.addr = cpu.esp;
    opr_dest.val = opr_src.val;
}
*/

static void instr_execute_2op()
{
    opr_dest.val = opr_src.addr;
    
    operand_write(&opr_dest);
}

make_instr_impl_2op(lea, rm, r, v)

