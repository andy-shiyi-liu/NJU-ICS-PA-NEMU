#ifndef __CACHE_H__
#define __CACHE_H__

#include "nemu.h"

#ifndef __MEMORY_H__
#include "memory/memory.h"
#endif

#define CACHE_UNIT_SIZE_INDEX 6 // 2^CACHE_UNIT_SIZE_INDEX Byte memory capacity within 1 line of cache

typedef struct CacheLine
{
    uint8_t data[1 << CACHE_UNIT_SIZE_INDEX]; // 64 Byte memory size
    uint32_t tag;
    bool valid;
} CacheLine;

#ifdef CACHE_ENABLED

// init the cache
void init_cache();

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data);

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len);

#endif

#endif
