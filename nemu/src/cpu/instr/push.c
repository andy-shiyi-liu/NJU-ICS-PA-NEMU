#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/

static void instr_execute_1op()
{
    operand_read(&opr_src);
    
    if(opr_src.data_size < data_size)
        sign_ext(opr_src.val, opr_src.data_size);

    opr_dest.type = OPR_MEM;
    opr_dest.data_size = data_size;
    cpu.esp -= opr_src.data_size / 8;
    opr_dest.addr = cpu.esp;
    opr_dest.val = opr_src.val;
    
    operand_write(&opr_dest);
}

//for Opcpode 50 ~ 57
make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, rm, v)
make_instr_impl_1op(push, i, v)


make_instr_func(push_i_b)
{
    opr_src.type = OPR_IMM;
    opr_src.addr = eip + 1;
    opr_src.sreg = SREG_CS;
    opr_src.data_size = 8;

    operand_read(&opr_src);

    //sign_ext(opr_src.val, opr_src.data_size);

    opr_dest.type = OPR_MEM;
    opr_dest.data_size = data_size;
    cpu.esp -= data_size / 8;
    opr_dest.addr = cpu.esp;
    opr_dest.val = opr_src.val;
    
    operand_write(&opr_dest);

    return 2;
}