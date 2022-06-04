#include "cpu/instr.h"

static void instr_execute_2op()
{
        operand_read(&opr_src);
        opr_dest.val = opr_src.val;
        operand_write(&opr_dest);
}

make_instr_impl_2op(mov, r, rm, b)

make_instr_impl_2op(mov, r, rm, v)

make_instr_impl_2op(mov, rm, r, b)

make_instr_impl_2op(mov, rm, r, v)

make_instr_impl_2op(mov, i, rm, b)

make_instr_impl_2op(mov, i, rm, v)

make_instr_impl_2op(mov, i, r, b)

make_instr_impl_2op(mov, i, r, v)

make_instr_impl_2op(mov, a, o, b)

make_instr_impl_2op(mov, a, o, v)

make_instr_impl_2op(mov, o, a, b)

make_instr_impl_2op(mov, o, a, v)

    // for mov gdt selector to segreg
make_instr_func(mov_rm2s_w)
{
        int len = 1;
        decode_data_size_w;
        decode_operand_rm2r;
        print_asm_2("mov", opr_dest.data_size == 8 ? "b" : (opr_dest.data_size == 16 ? "w" : "l"), len, &opr_src, &opr_dest);

        opr_dest.type = OPR_SREG;
        operand_read(&opr_src);
        opr_dest.val = opr_src.val;
        operand_write(&opr_dest);

        load_sreg(opr_dest.addr);

        return len;
}

// self write
make_instr_func(mov_r2c_l)
{
        OPERAND r, c;
        MODRM modrm;
        modrm.val = instr_fetch(eip + 1, 1);
        // printf("********************************\n");
        // printf("eip = %x\n", cpu.eip);
        // printf("modrm.val = %x\n", modrm.val);
        // printf("modrm.mod = %x\n", modrm.mod);
        // printf("modrm.reg = %x\n", modrm.reg_opcode);
        // printf("modrm.rm = %x\n", modrm.rm);
        
        r.addr = modrm.rm;
        r.data_size = 32;
        r.type = OPR_REG;
        r.sreg = SREG_CS;
        operand_read(&r);

        c.addr = modrm.reg_opcode;
        c.data_size = 32;
        c.type = OPR_CREG;
        r.sreg = SREG_CS;
        c.val = r.val;
        operand_write(&c);
        print_asm_2("mov","l", 2, &r, &c);

        return 2;
}

make_instr_func(mov_c2r_l)
{
        OPERAND r, c;
        MODRM modrm;
        modrm.val = instr_fetch(eip + 1, 1);
        // printf("********************************\n");
        // printf("eip = %x\n", cpu.eip);
        // printf("modrm.val = %x\n", modrm.val);
        // printf("modrm.mod = %x\n", modrm.mod);
        // printf("modrm.reg = %x\n", modrm.reg_opcode);
        // printf("modrm.rm = %x\n", modrm.rm);

        c.addr = modrm.reg_opcode;
        c.data_size = 32;
        c.type = OPR_CREG;
        r.sreg = SREG_CS;
        operand_read(&c);

        r.addr = modrm.rm;
        r.data_size = 32;
        r.type = OPR_REG;
        r.sreg = SREG_CS;
        r.val = c.val;
        operand_write(&r);

        return 2;
}

make_instr_func(mov_zrm82r_v)
{
        int len = 1;
        OPERAND r, rm;
        r.data_size = data_size;
        rm.data_size = 8;
        len += modrm_r_rm(eip + 1, &r, &rm);

        operand_read(&rm);
        r.val = rm.val;
        operand_write(&r);

        print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_zrm162r_l)
{
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);

        operand_read(&rm);
        r.val = rm.val;
        operand_write(&r);
        print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_srm82r_v)
{
        int len = 1;
        OPERAND r, rm;
        r.data_size = data_size;
        rm.data_size = 8;
        len += modrm_r_rm(eip + 1, &r, &rm);

        operand_read(&rm);
        r.val = sign_ext(rm.val, 8);
        operand_write(&r);
        print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_srm162r_l)
{
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);
        operand_read(&rm);
        r.val = sign_ext(rm.val, 16);
        operand_write(&r);

        print_asm_2("mov", "", len, &rm, &r);
        return len;
}
