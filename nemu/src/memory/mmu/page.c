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
	printf("**************************************\n");
	printf("addr.val = %x\n", addr.val);
	printf("addr.inPageAddr = %x\n", addr.inPageAddr);
	printf("addr.ptableIndex = %x\n", addr.ptableIndex);
	printf("addr.pdirIndex = %x\n", addr.pdirIndex);

	uint32_t pdirBaseAddr = cpu.cr3.pdbr;
	PDE pdir;
	pdir.val = paddr_read((pdirBaseAddr << 12) + 4 * addr.pdirIndex, 4);
	printf("pdir.val = %x\n", pdir.val);
	assert(pdir.present == 1);
	assert(pdir.read_write == 1);
	assert(pdir.user_supervisor == 1);

	uint32_t ptableBaseAddr = pdir.page_frame << 12;
	PTE ptable;
	ptable.val = paddr_read((ptableBaseAddr << 12) + 4 * addr.ptableIndex, 4);
	printf("ptable.val = %x\n", ptable.val);
	assert(ptable.present == 1);
	assert(ptable.read_write == 1);
	assert(ptable.user_supervisor == 1);

	uint32_t paddr = (ptable.page_frame << 12) + addr.inPageAddr;
	printf("paddr = %x\n", paddr);
	return paddr;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
