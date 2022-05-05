#include "cpu/instr.h"
/*
Put the implementations of `cmp' instructions here.
*/

static void instr_execute_2op() { // 所有cmp指令共享的执行方法
    operand_read(&opr_src);
    operand_read(&opr_dest);

    opr_src.val = sign_ext(opr_src.val, opr_src.data_size);

    alu_sub(opr_src.val, opr_dest.val, opr_dest.data_size);
}

/*
make_instr_func(cmp_i2rm_bv)
{
    int len = 1;
    decode_data_size_v
    
    opr_src.data_size = 8; //83 /7 ib   CMP r/m16,imm8    Compare sign extended immediate **byte** to r/m word
    
    //decode_operand, _, concat3(src_type, 2, dest_type)
    decode_operand_i2rm
    
    //print_asm_2(cmp_i2rm_v, opr_dest.data_size == 8 ? "b" : (opr_dest.data_size == 16 ? "w" : "l"), len, &opr_src, &opr_dest);
    operand_read(&opr_src);
    operand_read(&opr_dest);

    alu_sub(opr_src.val, opr_dest.val, opr_dest.data_size);
    return len;
}
*/

make_instr_impl_2op(cmp, i, rm, bv)
make_instr_impl_2op(cmp, r, rm, v)
make_instr_impl_2op(cmp, r, rm, b)
make_instr_impl_2op(cmp, i, rm, v)
make_instr_impl_2op(cmp, rm, r, v) //3b
make_instr_impl_2op(cmp, i, a, b) //3c
make_instr_impl_2op(cmp, i, rm, b) //80 7d
make_instr_impl_2op(cmp, rm, r, b) //3a
