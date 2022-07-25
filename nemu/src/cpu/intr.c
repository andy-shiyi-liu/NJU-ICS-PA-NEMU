#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
	// push EFLAGS
	opr_dest.type = OPR_MEM;
    opr_dest.data_size = 32;
    cpu.esp -= sizeof(cpu.eflags.val);
    opr_dest.addr = cpu.esp;
    opr_dest.val = cpu.eflags.val;
    
    operand_write(&opr_dest);

	// push CS
	opr_dest.type = OPR_MEM;
    opr_dest.data_size = 32;
    opr_dest.addr = cpu.esp;
    opr_dest.val = cpu.cs.val;
    cpu.esp -= sizeof(cpu.cs.val);
    
    operand_write(&opr_dest);

	// push EIP
	opr_dest.type = OPR_MEM;
    opr_dest.data_size = 32;
    opr_dest.addr = cpu.esp;
    opr_dest.val = cpu.eip;
    cpu.esp -= sizeof(cpu.eip);
    
    operand_write(&opr_dest);

	//find IDT entry
	uint32_t idtEntry = cpu.idtr.base + 4 * intr_no;

	//clear IF if it is a interrupt
	GateDesc idt;
	opr_src.type = OPR_MEM;
    opr_src.data_size = 32;
    opr_src.addr = idtEntry;
	operand_read(&opr_src);
	idt.val[0] = opr_src.val;

	opr_src.type = OPR_MEM;
    opr_src.data_size = 32;
    opr_src.addr = idtEntry + 4;
	operand_read(&opr_src);
	idt.val[1] = opr_src.val;

	if(idt.type == 14)
	{
		cpu.eflags.IF = 0;
	}

	printf("idt.val[0],[1]: %d, %d\n", idt.val[0], idt.val[1]);
	printf("idt.selector: %d\n", idt.selector);
	printf("cpu.cs.val: %d\n", cpu.cs.val);

	//set CS:EIP to the entry of the interrupt handler, need to reload CS with load_sreg()
	cpu.cs.val = idt.selector;
	cpu.eip = (idt.offset_31_16 << 16) + (idt.offset_15_0);
	load_sreg(1);
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
