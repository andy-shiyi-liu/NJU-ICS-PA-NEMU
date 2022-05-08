#include "cpu/instr.h"

make_instr_func(jmp_near)
{
        OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = data_size;
        rel.addr = eip + 1;

        operand_read(&rel);

        int offset = sign_ext(rel.val, data_size);
        // thank Ting Xu from CS'17 for finding this bug
        print_asm_1("jmp", "", 1 + data_size / 8, &rel);

        cpu.eip += offset;

        return 1 + data_size / 8;
}

make_instr_func(jmp_near_indirect)
{
        int len = 1;
        
        decode_operand_rm
        operand_read(&opr_src);

        if(data_size == 16)
        {
                cpu.eip = opr_src.val & 0x0000ffff;
        }
        else
        {
                cpu.eip = opr_src.val;
        }

        return 0;
}

make_instr_func(jmp_short)
{
        OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = 8;
        rel.addr = eip + 1;

        operand_read(&rel);

        int offset = sign_ext(rel.val, rel.data_size);
        // thank Ting Xu from CS'17 for finding this bug
        print_asm_1("jmp", "", 1 + 8 / 8, &rel);

        cpu.eip += offset;

        return 2;
}


make_instr_func(jmp_far_imm)
{
        opr_src.type = OPR_IMM; 
	opr_src.sreg = SREG_CS; 
	opr_src.addr = eip + 1; 
	len += opr_src.data_size / 8;
}