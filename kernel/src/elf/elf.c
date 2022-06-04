#include "common.h"
#include "memory.h"
#include "string.h"

#include <elf.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)

void ide_read(uint8_t *, uint32_t, uint32_t);
void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader()
{
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;

	uint32_t addr_shift = 0;
	uint8_t data;
	uint8_t *pdata;

#ifdef HAS_DEVICE_IDE
	uint8_t buf[4096];
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
	elf = (void *)buf;
	Log("ELF loading from hard disk.");
#else
	elf = (void *)0x0;
	Log("ELF loading from ram disk.");
#endif

	/* Load each program segment */
	ph = (void *)elf + elf->e_phoff;
	eph = ph + elf->e_phnum;
	for (; ph < eph; ph++)
	{
		if (ph->p_type == PT_LOAD)
		{

			// remove this panic!!!
			// panic("Please implement the loader");
#ifdef IA32_PAGE
			uint32_t paddrBase = mm_malloc(ph->p_vaddr, ph->p_memsz);
			for (addr_shift = 0; addr_shift < ph->p_filesz; addr_shift++)
			{
				// vaddr_write(ph->p_vaddr+addr_shift, 0, 1, vaddr_read(ph->p_offset+addr_shift, 0, 1));
				pdata = (void *)ph->p_offset + addr_shift;
				data = *pdata;
				pdata = (void *)paddrBase + addr_shift;
				*pdata = data;
			}
			for (addr_shift = ph->p_filesz; addr_shift < ph->p_memsz; addr_shift++)
			{
				// vaddr_write(ph->p_vaddr+addr_shift, 0, 1, 0);
				pdata = (void *)paddrBase + addr_shift;
				*pdata = 0;
			}
#else
			/* TODO: copy the segment from the ELF file to its proper memory area */
			for (addr_shift = 0; addr_shift < ph->p_filesz; addr_shift++)
			{
				// vaddr_write(ph->p_vaddr+addr_shift, 0, 1, vaddr_read(ph->p_offset+addr_shift, 0, 1));
				pdata = (void *)ph->p_offset + addr_shift;
				data = *pdata;
				pdata = (void *)ph->p_vaddr + addr_shift;
				*pdata = data;
			}
			/* TODO: zeror the memory area [vaddr + file_sz, vaddr + mem_sz) */
			for (addr_shift = ph->p_filesz; addr_shift < ph->p_memsz; addr_shift++)
			{
				// vaddr_write(ph->p_vaddr+addr_shift, 0, 1, 0);
				pdata = (void *)ph->p_vaddr + addr_shift;
				*pdata = 0;
			}
#endif
#ifdef IA32_PAGE
			/* Record the program break for future use */
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if (brk < new_brk)
			{
				brk = new_brk;
			}
#endif
		}
	}

	volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
	mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
#ifdef HAS_DEVICE_VGA
	create_video_mapping();
#endif
	write_cr3(get_ucr3());
#endif
	return entry;
}
