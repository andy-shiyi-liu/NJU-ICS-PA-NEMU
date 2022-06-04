#ifndef __INSTR_CMP_H__
#define __INSTR_CMP_H__
/*
Put the declarations of `cmp' instructions here.
*/

make_instr_func(cmp_i2rm_bv);
make_instr_func(cmp_r2rm_v); //39
make_instr_func(cmp_i2rm_v); //81 7d
make_instr_func(cmp_rm2r_v); //3b
make_instr_func(cmp_i2a_b);//3c
make_instr_func(cmp_i2rm_b);//80 7d
make_instr_func(cmp_rm2r_b);
make_instr_func(cmp_r2rm_b);
make_instr_func(cmp_i2a_v);

#endif
