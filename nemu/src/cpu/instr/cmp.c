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

make_instr_impl_2op(cmp, i, rm, bv)
make_instr_impl_2op(cmp, r, rm, v)
make_instr_impl_2op(cmp, r, rm, b)
make_instr_impl_2op(cmp, i, rm, v)
make_instr_impl_2op(cmp, rm, r, v) //3b
make_instr_impl_2op(cmp, i, a, b) //3c
make_instr_impl_2op(cmp, i, rm, b) //80 7d
make_instr_impl_2op(cmp, rm, r, b) //3a
make_instr_impl_2op(cmp, i, a, v) //3d
