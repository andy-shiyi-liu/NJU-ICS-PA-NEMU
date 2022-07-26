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

make_instr_func(in_v)
{
    if(data_size == 32)
    {
        cpu.eax = pio_read(cpu.edx, 4);
    }
    else if (data_size == 16)
    {
        cpu.eax = (cpu.eax & 0xffff0000) + pio_read(cpu.edx & 0xffff, 2);
    }
    else
    {
        printf("Data size error!\n");
        assert(0);
    }
    return 1;
}