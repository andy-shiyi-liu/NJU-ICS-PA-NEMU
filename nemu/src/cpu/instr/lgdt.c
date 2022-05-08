#include "cpu/instr.h"
/*
Put the implementations of `lgdt' instructions here.
*/

make_instr_func(lgdt)
{
    int len = 1;
    cpu.eip++; // marco: modrm_rm(eip + 1, &opr_src);
    decode_data_size_l;
    cpu.eip--;
    decode_operand_rm;
    print_asm_1("lgdt", opr_src.data_size == 8 ? "b" : (opr_src.data_size == 16 ? "w" : "l"), len, &opr_src);

    // lower 16 bit: limit
    cpu.gdtr.limit = vaddr_read(opr_src.addr, opr_src.sreg, 2);

    if (opr_src.data_size == 16) // higher 24 bit: base
    {
        cpu.gdtr.base = vaddr_read(opr_src.addr + 4, opr_src.sreg, 4) & 0x00ffffff;
    }
    else if (opr_src.data_size == 32)
    {
        cpu.gdtr.base = vaddr_read(opr_src.addr + 4, opr_src.sreg, 4);
    }
    else
    {
        assert("lgdt: data size ERROR!");
    }

    return len;
}