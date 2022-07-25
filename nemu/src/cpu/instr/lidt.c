#include "cpu/instr.h"
/*
Put the implementations of `lidt' instructions here.
*/
make_instr_func(lidt)
{
    int len = 1;
    cpu.eip++; // marco: modrm_rm(eip + 1, &opr_src);
    decode_data_size_l;
    cpu.eip--;
    decode_operand_rm;

    // lower 16 bit: limit
    cpu.idtr.limit = laddr_read(opr_src.addr, 2);

    if (opr_src.data_size == 16) // higher 24 bit: base
    {
        cpu.idtr.base = laddr_read(opr_src.addr + 2, 4) & 0x00ffffff;
    }
    else if (opr_src.data_size == 32)
    {
        cpu.idtr.base = laddr_read(opr_src.addr + 2, 4);
    }
    else
    {
        assert("lidt: data size ERROR!");
    }

    print_asm_1("lidt", opr_src.data_size == 8 ? "b" : (opr_src.data_size == 16 ? "w" : "l"), len, &opr_src);
    return len;
}