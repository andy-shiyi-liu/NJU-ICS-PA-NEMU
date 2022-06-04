#include "cpu/cpu.h"
#include "memory/memory.h"

typedef union 
{
	struct
	{
		uint32_t inPageAddr :12;
		uint32_t ptableIndex :10;
		uint32_t pdirIndex :10;
	};
	uint32_t val;
}LADDR;


// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
	LADDR addr;
	addr.val = laddr;
	// printf("**************************************\n");
	// printf("addr.val = %x\n", addr.val);
	// printf("addr.inPageAddr = %x\n", addr.inPageAddr);
	// printf("addr.ptableIndex = %x\n", addr.ptableIndex);
	// printf("addr.pdirIndex = %x\n", addr.pdirIndex);

	uint32_t pdirBaseAddr = cpu.cr3.pdbr << 12;
	PDE pdir;
	// printf("addr: %x\n", pdirBaseAddr + sizeof(PDE) * addr.pdirIndex);
	pdir.val = paddr_read(pdirBaseAddr + sizeof(PDE) * addr.pdirIndex, sizeof(PDE));
	// printf("pdir.val = %x\n", pdir.val);
	// printf("pdir.page_frame = %x\n", pdir.page_frame);
	assert(pdir.present == 1);
	assert(pdir.read_write == 1);
	assert(pdir.user_supervisor == 1);

	uint32_t ptableBaseAddr = pdir.page_frame << 12;
	PTE ptable;
	// printf("addr: %x\n", ptableBaseAddr + sizeof(PTE) * addr.ptableIndex);
	ptable.val = paddr_read(ptableBaseAddr + sizeof(PTE) * addr.ptableIndex, sizeof(PTE));
	// printf("ptable.val = %x\n", ptable.val);
	// printf("ptable.page_frame = %x\n", ptable.page_frame);
	assert(ptable.present == 1);
	assert(ptable.read_write == 1);
	assert(ptable.user_supervisor == 1);

	uint32_t paddr = (ptable.page_frame << 12) + addr.inPageAddr;
	// printf("paddr = %x\n", paddr);
	return paddr;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
