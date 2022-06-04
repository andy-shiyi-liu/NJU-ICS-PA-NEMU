#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/

make_instr_func(ret_near)
{
    //opr_src: stack value to be writen to %eip
    //pop
    opr_src.type = OPR_MEM;
    opr_src.data_size = data_size;
    opr_src.addr = cpu.esp;
    operand_read(&opr_src);
    cpu.esp += opr_src.data_size / 8;
    
    //set %eip
    cpu.eip = opr_src.val;

    print_asm_0("ret", "", 1);
    return 0;
}

make_instr_func(ret_near_imm16)
{
    // opr_src: immediate val
    opr_src.type = OPR_IMM;
    opr_src.data_size = 16;
    opr_src.addr = cpu.eip + 1;
    operand_read(&opr_src);

    //opr_dest: stack value to be writen to %eip
    //pop
    opr_dest.type = OPR_MEM;
    opr_dest.data_size = data_size;
    opr_dest.addr = cpu.esp;
    operand_read(&opr_dest);
    cpu.esp += opr_dest.data_size / 8;
    
    //set %eip
    cpu.eip = opr_dest.val;
    if(data_size == 16)
        cpu.eip = (cpu.eip & 0x0000ffff);

    //set %esp
    cpu.esp += opr_src.val;
    
    print_asm_1("ret", "", 4, &opr_src);
    return 0;
}