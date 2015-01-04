/*
 * libci20 Firmware
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>

#include "cache.h"

#define DCACHE_SIZE		32768
#define DCACHE_LINE		32

#define ICACHE_SIZE		32768
#define ICACHE_LINE		32

#define INDEX_STORE_TAG_D	0x09
#define INDEX_STORE_TAG_I	0x08
#define HIT_INVALIDATE_I	0x10
#define HIT_WRITEBACK_INV_D	0x15

#define cache_op __builtin_mips_cache

static inline void write_taglo(uint32_t val)
{
	asm volatile("mtc0	%0, $28" : : "r"(val));
}

static inline void write_taghi(uint32_t val)
{
	asm volatile("mtc0	%0, $29" : : "r"(val));
}

void init_icache(void)
{
	const volatile void *addr = (void *)0x80000000;
	const volatile void *end = addr + ICACHE_SIZE;

	write_taglo(0);
	write_taghi(0);

	for (; addr < end; addr += ICACHE_LINE)
		cache_op(INDEX_STORE_TAG_I, addr);
}

void flush_icache(uintptr_t base, size_t size)
{
	const volatile void *addr = (void *)(base & ~(ICACHE_LINE - 1));
	const volatile void *end = (void *)((base + size) & ~(ICACHE_LINE - 1));

	for (; addr <= end; addr += ICACHE_LINE)
		cache_op(HIT_INVALIDATE_I, addr);
}

void init_dcache(void)
{
	const volatile void *addr = (void *)0x80000000;
	const volatile void *end = addr + DCACHE_SIZE;

	write_taglo(0);
	write_taghi(0);

	for (; addr < end; addr += DCACHE_LINE)
		cache_op(INDEX_STORE_TAG_I, addr);
}

void flush_dcache(uintptr_t base, size_t size)
{
	const volatile void *addr = (void *)(base & ~(DCACHE_LINE - 1));
	const volatile void *end = (void *)((base + size) & ~(DCACHE_LINE - 1));

	for (; addr <= end; addr += DCACHE_LINE)
		cache_op(HIT_WRITEBACK_INV_D, addr);
}
