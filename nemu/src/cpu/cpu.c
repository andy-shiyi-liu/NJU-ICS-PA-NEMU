#include "nemu.h"
#include "cpu/cpu.h"
#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"
#include "monitor/breakpoint.h"
#ifndef __STDIO_H_
#include <stdio.h>
#endif
#include <assert.h>

CPU_STATE cpu;
FPU fpu;
int nemu_state;
uint8_t data_size = 32;
bool verbose = false;
bool is_nemu_hlt = false;
bool has_prefix = false;

#define sign(x) ((uint32_t)(x) >> 31)

void do_devices();
void init_cpu(const uint32_t init_eip)
{
	cpu.eflags.val = 0x0;
	fpu.status.val = 0x0;
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		cpu.gpr[i].val = 0x0;
		fpu.regStack[i].val = 0x0;
	}
	cpu.eip = init_eip;
	cpu.esp = (128 << 20) - 16;
#ifdef IA32_SEG
	cpu.cr0.val = 0x0;
	cpu.gdtr.base = cpu.gdtr.limit = 0x0;
	for (i = 0; i < 6; i++)
	{
		cpu.segReg[i].val = 0x0;
	}
#endif
#ifdef IA32_INTR
	cpu.idtr.base = cpu.idtr.limit = 0x0;
	cpu.intr = 0x0;
	i8259_init();
#endif
}


void exec(uint32_t n)
{
	static BP *bp = NULL;
	verbose = (n <= 100000);
	int instr_len = 0;
	bool hit_break_rerun = false;

	if (nemu_state == NEMU_BREAK)
	{
		hit_break_rerun = true;
	}

	nemu_state = NEMU_RUN;
	while (n > 0 && nemu_state == NEMU_RUN)
	{
		if(!is_nemu_hlt)
		{
			instr_len = exec_inst();
			cpu.eip += instr_len;
			n--;

			// printf("cpu.esp: %x\n", cpu.esp);
			// printf("cpu.eax: %x\n", cpu.eax);
			// printf("cpu.ebx: %x\n", cpu.ebx);
			// printf("cpu.ebp: %x\n", cpu.ebp);
			// printf("cpu.ecx: %x\n", cpu.ecx);
			// printf("cpu.edi: %x\n", cpu.edi);
			// printf("cpu.edx: %x\n", cpu.edx);
			// printf("cpu.esi: %x\n", cpu.esi);
			// printf("cpu.eflags.AF: %x\n", cpu.eflags.AF);
			// printf("cpu.eflags.CF: %x\n", cpu.eflags.CF);
			// printf("cpu.eflags.IF: %x\n", cpu.eflags.IF);
			// printf("cpu.eflags.NT: %x\n", cpu.eflags.NT);
			// printf("cpu.eflags.OF: %x\n", cpu.eflags.OF);
			// printf("cpu.eflags.OLIP: %x\n", cpu.eflags.OLIP);
			// printf("cpu.eflags.PF: %x\n", cpu.eflags.PF);
			// printf("cpu.eflags.RF: %x\n", cpu.eflags.RF);
			// printf("cpu.eflags.SF: %x\n", cpu.eflags.SF);
			// printf("cpu.eflags.TF: %x\n", cpu.eflags.TF);
			// printf("cpu.eflags.VM: %x\n", cpu.eflags.VM);
			// printf("cpu.eflags.ZF: %x\n", cpu.eflags.ZF);
			// printf("cpu.cr0.val: %x\n", cpu.cr0.val);
			// printf("cpu.es: %x\n", cpu.es.val);
			// printf("cpu.cs: %x\n", cpu.cs.val);
			// printf("cpu.ss: %x\n", cpu.ss.val);
			// printf("cpu.ds: %x\n", cpu.ds.val);
			// printf("cpu.fs: %x\n", cpu.fs.val);
			// printf("cpu.gs: %x\n", cpu.gs.val);
			// printf("\n*************************************\n\n");
			// printf("cpu.eip: %x\n", cpu.eip);

			if (hit_break_rerun)
			{
				resume_breakpoints();
				hit_break_rerun = false;
			}

			// check for breakpoints
			if (nemu_state == NEMU_BREAK)
			{
				// find break in the list
				bp = find_breakpoint(cpu.eip - 1);
				if (bp)
				{
					// found, then restore the original opcode
					vaddr_write(bp->addr, SREG_CS, 1, bp->ori_byte);
					cpu.eip--;
				}
				// not found, it is triggered by BREAK_POINT in the code, do nothing
			}

			// check for watchpoints

			BP *wp = scan_watchpoint();
			if (wp != NULL)
			{
				// print_bin_instr(eip_temp, instr_len);
				// puts(assembly);
				printf("\n\nHit watchpoint %d at address 0x%08x, expr = %s\n", wp->NO, cpu.eip - instr_len, wp->expr);
				printf("old value = %#08x\nnew value = %#08x\n", wp->old_val, wp->new_val);
				wp->old_val = wp->new_val;
				nemu_state = NEMU_READY;
				break;
			}
		}
#if defined(HAS_DEVICE_TIMER) || defined(HAS_DEVICE_VGA) || defined(HAS_DEVICE_KEYBOARD) || defined(HAS_DEVICE_AUDIO)
	do_devices();
#endif
#ifdef IA32_INTR
		// check for interrupt
		do_intr();
#endif
	}
	if (nemu_state == NEMU_STOP)
	{
		printf("NEMU2 terminated\n");
#ifdef IA32_INTR
		i8259_destroy();
#endif
	}
	else if (n == 0)
	{
		nemu_state = NEMU_READY;
	}
}

int exec_inst()
{
	uint8_t opcode = 0;
	// get the opcode
	opcode = instr_fetch(cpu.eip, 1);
//printf("opcode = %x, eip = %x\n", opcode, cpu.eip);
// instruction decode and execution
#ifdef NEMU_REF_INSTR
	int len = __ref_opcode_entry[opcode](cpu.eip, opcode);
#else
	int len = opcode_entry[opcode](cpu.eip, opcode);
#endif
	return len;
}

#ifdef IA32_INTR
// do interrupt
void do_intr()
{
	if (cpu.intr && cpu.eflags.IF)
	{
		is_nemu_hlt = false;
		// get interrupt number
		uint8_t intr_no = i8259_query_intr_no(); // get interrupt number
		assert(intr_no != I8259_NO_INTR);
		i8259_ack_intr();	// tell the PIC interrupt info received
		raise_intr(intr_no); // raise intrrupt to turn into kernel handler
	}
}
#endif
