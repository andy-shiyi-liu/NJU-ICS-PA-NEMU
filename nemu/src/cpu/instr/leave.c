#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/

make_instr_func(leave)
{
    cpu.esp = cpu.ebp;

    opr_dest.type = OPR_MEM;
    opr_dest.data_size = data_size;
    opr_dest.addr = cpu.esp;
    operand_read(&opr_dest);
    cpu.esp += data_size / 8;
    
    cpu.ebp = opr_dest.val;

    print_asm_0("lea", "", 1);
    return 1;
}

