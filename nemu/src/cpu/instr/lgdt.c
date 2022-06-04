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

    // lower 16 bit: limit
    cpu.gdtr.limit = laddr_read(opr_src.addr, 2);

    if (opr_src.data_size == 16) // higher 24 bit: base
    {
        cpu.gdtr.base = laddr_read(opr_src.addr + 2, 4) & 0x00ffffff;
    }
    else if (opr_src.data_size == 32)
    {
        cpu.gdtr.base = laddr_read(opr_src.addr + 2, 4);
    }
    else
    {
        assert("lgdt: data size ERROR!");
    }

    print_asm_1("lgdt", opr_src.data_size == 8 ? "b" : (opr_src.data_size == 16 ? "w" : "l"), len, &opr_src);
    return len;
}