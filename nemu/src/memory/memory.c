#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "device/mm_io.h"
#include <memory.h>
#include <stdio.h>

uint8_t hw_mem[MEM_SIZE_B];

static inline uint32_t mask_byte(uint32_t data, uint32_t byte)
{
	switch (byte)
	{
	case 1:
		return data & 0xff;
		break;
	case 2:
		return data & 0xffff;
		break;
	case 3:
		return data & 0xffffff;
		break;
	default:
		printf("data len ERROR!");
		assert(0);
		break;
	}
}

uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
#ifdef CACHE_ENABLED
	ret = cache_read(paddr, len);
#else
	ret = hw_mem_read(paddr, len);
#endif
	return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data)
{
#ifdef CACHE_ENABLED
	cache_write(paddr, len, data);
#else
	hw_mem_write(paddr, len, data);
#endif
}

uint32_t laddr_read(laddr_t laddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	if (cpu.cr0.pe == 1 && cpu.cr0.pg == 1)
	{
		if (laddr >> 12 != (laddr + len -1) >> 12)
		{
			// printf("Cross-page Data Read!\n");
			// printf("laddr: %x\n", laddr);
			// printf("len: %x\n", len);
			// printf("laddr + len: %x\n", laddr + len -1);

			uint8_t nextPageLen = (laddr + len) & 0xfff;
			uint8_t inPageLen = len - nextPageLen;
			// printf("inPageLen: %x\n", inPageLen);
			// printf("nextPageLen: %x\n", nextPageLen);

			uint32_t inPagePaddr = page_translate(laddr);
			uint32_t inPageData = paddr_read(inPagePaddr, len);
			inPageData = mask_byte(inPageData, inPageLen);
			
			uint32_t nextPagePaddr = page_translate(laddr + inPageLen);
			uint32_t nextPageData = paddr_read(nextPagePaddr, len);
			nextPageData = mask_byte(nextPageData, nextPageLen);

			// printf("inPageData: %x\n", inPageData);
			// printf("nextPageData: %x\n", nextPageData);

			uint32_t data = (nextPageData << (8 * inPageLen)) + inPageData;
			// printf("data: %x\n", data);
			return data;
		}
		else
		{
			uint32_t paddr = page_translate(laddr);
			return paddr_read(paddr, len);
		}
	}
	else
	{
		return paddr_read(laddr, len);
	}
}

void laddr_write(laddr_t laddr, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
	if(cpu.cr0.pe == 1 && cpu.cr0.pg == 1){
		if (laddr >> 12 != (laddr + len -1) >> 12)
		{
			printf("Cross-page Data Write!\n");
			printf("laddr: %x\n", laddr);
			printf("len: %x\n", len);
			printf("laddr + len: %x\n", laddr + len);
			assert(0);
		}
		else
		{
			uint32_t paddr = page_translate(laddr);
			paddr_write(paddr, len, data);
		}
	}
	else{
		paddr_write(laddr, len, data);
	}
	
}

uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
#ifndef IA32_SEG
	return laddr_read(vaddr, len);
#else
	uint32_t laddr = vaddr;
	if (cpu.cr0.pe == 1)
	{
		laddr = segment_translate(vaddr, sreg);
	}
	return laddr_read(laddr, len);
#endif
}

void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
#ifndef IA32_SEG
	laddr_write(vaddr, len, data);
#else
	uint32_t laddr = vaddr;
	if (cpu.cr0.pe == 1)
	{
		laddr = segment_translate(vaddr, sreg);
	}
	laddr_write(laddr, len, data);
#endif
}

void init_mem()
{
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);

#ifdef CACHE_ENABLED
	init_cache();
#endif

#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif
}

uint32_t instr_fetch(vaddr_t vaddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t *get_mem_addr()
{
	return hw_mem;
}
