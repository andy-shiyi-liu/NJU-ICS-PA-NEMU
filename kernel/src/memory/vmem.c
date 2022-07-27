#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)
#define NR_PT ((SCR_SIZE + PT_SIZE - 1) / PT_SIZE) // number of page tables to cover the vmem

PDE *get_updir();

void create_video_mapping()
{

	/* TODO: create an identical mapping from virtual memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memeory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */

	// panic("please implement me");

	// Log("Creating video mapping, magic 5");
	PDE *updir = get_updir();
	uint32_t uptable_idx, upframe_idx;
	// Log("Fetched PDE");

	/* make all PDE invalid */
	memset(updir, 0, NR_PT * sizeof(PDE));

	/* fill PDEs and PTEs */
	updir->val = make_pde(updir->page_frame);
	upframe_idx = 160;

	PTE *uptable = (PTE *)(((updir->page_frame << 12) & 0xfffff000) + upframe_idx * sizeof(PTE));
	for (uptable_idx = 0; uptable_idx < (SCR_SIZE / 4096) + 1; uptable_idx++)
	{
		uptable->val = make_pte(upframe_idx << 12);
		// Log("make pte of ptable addr: %x, page fame index: %x, page frame addr: %x", (uint32_t)uptable, upframe_idx, uptable->page_frame);
		upframe_idx++;
		uptable++;
	}
	// Log("Finished Video mapping");
}

void video_mapping_write_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		buf[i] = i;
	}
}

void video_mapping_read_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		assert(buf[i] == i);
	}
}

void video_mapping_clear()
{
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}
