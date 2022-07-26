#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t inline segment_translate(uint32_t offset, uint8_t sreg)
{
	/* TODO: perform segment translation from virtual address to linear address
	 * by reading the invisible part of the segment register 'sreg'
	 */

	uint32_t result = 0;

	result = cpu.segReg[sreg].base + offset;

	assert(result < cpu.segReg[sreg].limit);

	return result;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
	/* TODO: load the invisibile part of the segment register 'sreg' by reading the GDT.
	 * The visible part of 'sreg' should be assigned by mov or ljmp already.
	 */
	// printf("load sreg %d\n", sreg);
	// printf("cpu.gdtr.base: %d\n", cpu.gdtr.base);
	uint32_t segDescriptorAddr = cpu.gdtr.base + 8 * cpu.segReg[sreg].index;
	// printf("segDescriptorAddr: %d\n", segDescriptorAddr);
	if(segDescriptorAddr > cpu.gdtr.base + cpu.gdtr.limit)
	{
		printf("ERROR: gdt table overflow!");
		assert(0);
	}
	uint32_t entityVal;
	
	// SegDesc *psegDescriptor = (SegDesc*)&hw_mem[segDescriptorAddr];
	SegDesc segDescriptor;
	segDescriptor.val[0] = laddr_read(segDescriptorAddr, 4);
	segDescriptor.val[1] = laddr_read(segDescriptorAddr + 4, 4);
	SegDesc *psegDescriptor = &segDescriptor;

	assert(psegDescriptor->granularity == 1);

	// base
	entityVal = psegDescriptor->base_31_24 << 24; // B31~B24

	entityVal += psegDescriptor->base_23_16 << 16;   // B23~B26

	entityVal += psegDescriptor->base_15_0;  // B15~B0

	assert(entityVal == 0);
	cpu.segReg[sreg].base = entityVal;
	// printf("entityVal: %x\n", entityVal);
	// printf("cpu.segReg[sreg].base: %x\n", cpu.segReg[sreg].base);

	// limit
	entityVal = psegDescriptor->limit_19_16 << 16; 
	entityVal += psegDescriptor->limit_15_0;
	assert(entityVal == 0xfffff);

	if(psegDescriptor->granularity == 1){
		entityVal = entityVal << 12;
	}
	cpu.segReg[sreg].limit = entityVal;
	// printf("entityVal: %x\n", entityVal);
	// printf("cpu.segReg[sreg].limit: %x\n", cpu.segReg[sreg].limit);

	// type
	entityVal = psegDescriptor->type;

	cpu.segReg[sreg].type = entityVal;
	// printf("entityVal: %x\n", entityVal);
	// printf("cpu.segReg[sreg].type: %x\n", cpu.segReg[sreg].type);

	// privilege_level
	entityVal = psegDescriptor->privilege_level;

	cpu.segReg[sreg].privilege_level = entityVal;
	// printf("entityVal: %x\n", entityVal);
	// printf("cpu.segReg[sreg].privilege_level: %x\n", cpu.segReg[sreg].privilege_level);

	// soft_use
	entityVal = psegDescriptor->soft_use;

	cpu.segReg[sreg].soft_use = entityVal;
	// printf("entityVal: %x\n", entityVal);
	// printf("cpu.segReg[sreg].soft_use: %x\n", cpu.segReg[sreg].soft_use);
}
