#include "memory/mmu/cache.h"
#ifndef _STDLIB_H
#include <stdlib.h>
#endif
#ifndef _STDIO_H
#include <stdio.h>
#endif

#define CACHE_LINE_TOTAL_NUM 1024
#define SET_ASSO_NUM 8

struct CacheLine cache[CACHE_LINE_TOTAL_NUM];

static uint32_t get_in_block_addr(paddr_t paddr)
{
	return paddr & 0x0000003f; // get lower 6 bit
}

static uint32_t get_block_addr(paddr_t paddr)
{
	return paddr >> CACHE_UNIT_SIZE_INDEX; // get higher 32-6 bit
}

static uint32_t get_cache_group_addr(paddr_t paddr)
{
	paddr = paddr >> 6;
	return paddr & 0x0000007f; // get lower [7, 13] bit
}

static uint32_t get_tag(paddr_t paddr)
{
	paddr = paddr >> 13;
	return paddr; // get higher 32-13 bit
}

static int32_t get_line_num(uint32_t groupNum, uint32_t tag) // return cache line addr if hit, otherwise return -1
{
	int32_t lineNum;
	for (lineNum = SET_ASSO_NUM * groupNum; lineNum < SET_ASSO_NUM * (groupNum + 1); lineNum++)
	{
		if (cache[lineNum].valid == true && cache[lineNum].tag == tag)
		{
			return lineNum;
		}
	}
	return -1;
}

static int32_t get_empty_line_num(uint32_t groupNum) // return the line num of a invalid line in a group, return -1 if the cache is full.
{
	int32_t lineNum;
	for (lineNum = SET_ASSO_NUM * groupNum; lineNum < SET_ASSO_NUM * (groupNum + 1); lineNum++)
	{
		if (cache[lineNum].valid == false)
		{
			return lineNum;
		}
	}
	return -1;
}

static uint32_t get_random_replace_line_num(uint32_t groupNum)
{
	uint32_t randNum = rand();
	randNum = randNum % SET_ASSO_NUM;
	randNum = randNum + SET_ASSO_NUM * groupNum;
	return randNum;
}

static void load_cache_line(int32_t lineNum, uint32_t tag, uint32_t paddr)
{
	cache[lineNum].valid = true;
	cache[lineNum].tag = tag;

	// find the corrsponding start addr of the block
	paddr = paddr >> CACHE_UNIT_SIZE_INDEX;
	paddr = paddr << CACHE_UNIT_SIZE_INDEX;

	for (uint32_t i = 0; i < (1 << CACHE_UNIT_SIZE_INDEX); i++)
	{
		cache[lineNum].data[i] = hw_mem_read(paddr + i, 1);
	}
}

static uint32_t replace_line_data(uint32_t paddr, uint32_t groupNum, uint32_t tag) // find a empty line to load mem data, if cache full, randomly select a line in the gruop to replace.
{
	int32_t emptyLineNum = get_empty_line_num(groupNum);
	if (emptyLineNum == -1) // cache full
	{
		emptyLineNum = get_random_replace_line_num(groupNum);
	}
	load_cache_line(emptyLineNum, tag, paddr);
	return emptyLineNum;
}

static uint32_t read_cache_data(int32_t lineNum, uint32_t inBlockAddr, uint32_t len)
{
	uint32_t result = 0;
	int32_t i = 0;
	for (i = len - 1; i >= 0; i--) // caution: little endian!!
	{
		if (inBlockAddr + i > (1 << CACHE_UNIT_SIZE_INDEX)) // cross line data
		{
			assert("attempting to read cross line data!");
		}
		result = result << 8; // 8: byte to bit
		result += cache[lineNum].data[inBlockAddr + i];
	}
	return result;
}

static void write_cache_data(int32_t lineNum, uint32_t inBlockAddr, uint32_t data, uint32_t len)
{
	uint32_t i = 0;
	for (i = 0; i < len; i++)
	{
		if (inBlockAddr > (1 << CACHE_UNIT_SIZE_INDEX)) // cross line data
		{
			assert("attempting to read cross line data!");
		}
		assert(lineNum < CACHE_LINE_TOTAL_NUM);
		cache[lineNum].data[inBlockAddr] = data & 0xff; // 0xff: write 1 byte data
		data = (data >> 8);								// 8: byte to bit.
		inBlockAddr++;
	}
}

// init the cache
void init_cache()
{
	// implement me in PA 3-1
	uint32_t i;
	for (i = 0; i < CACHE_LINE_TOTAL_NUM; i++)
	{
		cache[i].valid = false;
	}
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	// implement me in PA 3-1
	assert(len <= 4 || len >= 0);
	if (len == 0)
	{
		return;
	}

	uint32_t tag = get_tag(paddr);
	uint32_t inBlockAddr = get_in_block_addr(paddr);
	uint32_t groupNum = get_cache_group_addr(paddr);

	int32_t lineNum = get_line_num(groupNum, tag);

	// case 1: memory access within 1 line
	if (get_block_addr(paddr) == get_block_addr(paddr + len - 1))
	{
		if (lineNum != -1) // if cache hit, update cache data
		{
			write_cache_data(lineNum, inBlockAddr, data, len);
		}
		// write through, write mem data for all time.
		hw_mem_write(paddr, len, data);
		// uint32_t memData = hw_mem_read(paddr, len);
		// assert(memData == data);
	}
	else // case 2: cross-line memory access
	{
		int32_t inBlockLen = (1 << CACHE_UNIT_SIZE_INDEX) - inBlockAddr;
		int32_t nextBlockLen = len - inBlockLen;

		//assert(inBlockLen > 0 && inBlockLen < len);
		//assert(nextBlockLen > 0);

		uint32_t inBlockData = (data & (0xffffffff >> (32 - 8 * inBlockLen)));
		uint32_t nextBlockData = (data & (0xffffffff << (8*inBlockLen))) >>(8*inBlockLen); 

		// little endian
		cache_write(paddr, inBlockLen, inBlockData);
		// uint32_t mask = 0xffffffff;
		// mask = mask >> (32 - (inBlockLen * 8));
		// uint32_t memData = hw_mem_read(paddr, len) & mask;
		// uint32_t maskedData = data & mask;
		// assert(memData == maskedData);
		cache_write(paddr + inBlockLen, nextBlockLen, nextBlockData);
		// memData = hw_mem_read(paddr, len);
		// assert(memData == data);
	}
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
	// implement me in PA 3-1
	assert(len <= 4 || len >= 0);
	if (len == 0)
	{
		return 0;
	}

	uint32_t tag = get_tag(paddr);
	uint32_t inBlockAddr = get_in_block_addr(paddr);
	uint32_t groupNum = get_cache_group_addr(paddr);

	int32_t lineNum = get_line_num(groupNum, tag);
	uint32_t result = 0;

	if (lineNum == -1) // cache not hit, load cache first and update loaded lineNum
	{
		lineNum = replace_line_data(paddr, groupNum, tag);
	}

	// case 1: memory access within 1 line
	if (get_block_addr(paddr) == get_block_addr(paddr + len - 1))
	{
		result = read_cache_data(lineNum, inBlockAddr, len);

		// assert(result == hw_mem_read(paddr, len));
		return result;
	}
	else // case 2: cross-line memory access
	{
		int32_t inBlockDataLen = (1 << CACHE_UNIT_SIZE_INDEX) - inBlockAddr;
		int32_t nextBlockDataLen = len - inBlockDataLen;

		//assert(inBlockDataLen > 0 && inBlockDataLen < len);
		//assert(nextBlockDataLen > 0);

		// little endian
		result = cache_read(paddr + inBlockDataLen, nextBlockDataLen); // higher bits
		result = result << 8 * inBlockDataLen;						   // 8: byte to bit
		result += cache_read(paddr, inBlockDataLen);				   // lower bit

		// assert(result == hw_mem_read(paddr, len));
		return result;
	}
	// should not reach here
	assert(0);
}
