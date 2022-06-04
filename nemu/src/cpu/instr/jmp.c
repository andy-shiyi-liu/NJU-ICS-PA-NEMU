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
        opr_src.sreg = SREG_CS;
        operand_read(&opr_src);
        if (data_size == 16)
        {
                cpu.eip = opr_src.val & 0x0000ffff;
        }
        else
        {
                cpu.eip = opr_src.val;
        }
        print_asm_1("jmp", "", len, &opr_src);
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
        OPERAND eipOp, csOp;

        eipOp.type = OPR_IMM;
        eipOp.sreg = SREG_CS;
        eipOp.addr = eip + 1;
        eipOp.data_size = data_size;

        csOp.type = OPR_IMM;
        csOp.sreg = SREG_CS;
        csOp.addr = eip + 1 + data_size / 8;
        csOp.data_size = 16;

        operand_read(&eipOp);
        operand_read(&csOp);

        cpu.cs.val = csOp.val;
        cpu.eip = eipOp.val;

        load_sreg(1);

        print_asm_2("jmp", "", (data_size+16) / 8, &opr_src, &opr_dest);

        return 0;
}