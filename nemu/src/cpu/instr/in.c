#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `in' instructions here.
*/

make_instr_func(in_b)
{
    cpu.eax = (cpu.eax & 0xffffff00) + pio_read(cpu.edx & 0xffff, 1);
    return 1;
}