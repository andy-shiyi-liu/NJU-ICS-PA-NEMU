#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/

make_instr_func(call_near)
{
    //printf("cpu.eip: %x\n", cpu.eip);
    
    //opr_src: imm rel addr
    opr_src.type = OPR_IMM;
    opr_src.data_size = 32;
    opr_src.addr = cpu.eip+1;
    operand_read(&opr_src);
    //printf("opr_src.val: %x\n", opr_src.val);
    
    //opr_dest: val=eip+5 (addr of next instr), to be pushed to stack
    cpu.eip += 5;
    opr_dest.type = OPR_MEM;
    opr_dest.data_size = 32;
    cpu.esp -= 4;
    opr_dest.addr = cpu.esp;
    opr_dest.val = cpu.eip;
    
    operand_write(&opr_dest); //write %eip to stack
    
    //set %eip
    cpu.eip += opr_src.val;
    return 0;
}

make_instr_func(call_near_indirect)
{
    //printf("cpu.eip: %x\n", cpu.eip);
    
    int len = 0;
    //opr_src: Ev
    decode_operand_rm
    operand_read(&opr_src);
    //printf("opr_src.val: %x\n", opr_src.val);
    
    //opr_dest: val=eip+5 (addr of next instr), to be pushed to stack
    cpu.eip += 2;
    opr_dest.type = OPR_MEM;
    opr_dest.data_size = 32;
    cpu.esp -= 4;
    opr_dest.addr = cpu.esp;
    opr_dest.val = cpu.eip;
    
    operand_write(&opr_dest); //write %eip to stack
    
    //set %eip
    cpu.eip = opr_src.val;

    if(data_size == 16)
    {
        cpu.eip &= 0x0000ffff;
    }

    return 0;
}
