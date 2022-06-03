#include "cpu/cpu.h"
#include "memory/memory.h"

static inline uint32_t get_pdir_index(laddr_t laddr)
{
	return laddr >> 22; // 31~22 bit
}

static inline uint32_t get_ptable_index(laddr_t laddr)
{
	return (laddr >> 12) & 0x3ff; // 21~12 bit
}

static inline uint32_t get_inpage_addr(laddr_t laddr)
{
	return laddr & 0xfff;
}

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
	uint32_t pdirIndex = get_pdir_index(laddr);
	uint32_t ptableIndex = get_ptable_index(laddr);
	uint32_t inPageAddr = get_inpage_addr(laddr);

	uint32_t pdirBaseAddr = cpu.cr3.pdbr;
	PDE *pdir = (PDE*)(hw_mem + (pdirBaseAddr << 12));
	assert(pdir->present == 1);
	assert(pdir->read_write == 1);
	assert(pdir->user_supervisor == 1);

	uint32_t ptableBaseAddr = pdir[pdirIndex].page_frame;
	PTE *ptable = (PTE*)(hw_mem + (ptableBaseAddr << 12));
	assert(ptable->present == 1);
	assert(ptable->read_write == 1);
	assert(ptable->user_supervisor == 1);

	return (ptable[ptableIndex].page_frame << 12) + inPageAddr;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
