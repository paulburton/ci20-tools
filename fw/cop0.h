/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_COP0_H__
#define __FW_COP0_H__

#include <stdint.h>

#include "cache.h"

static inline uint32_t dynamic_mfc0(unsigned reg, unsigned sel)
{
	uint32_t code[3] = {
		0x40020000,		/* mfc0 v0, X */
		0x03e00008,		/* jr ra */
		0x00000000,		/*  nop */
	};
	uint32_t (*fn)(void) = (void *)code;

	code[0] |= reg << 11;
	code[0] |= sel << 0;

	flush_dcache((uintptr_t)code, sizeof(code));
	flush_icache((uintptr_t)code, sizeof(code));

	return fn();
}

static inline void dynamic_mtc0(unsigned reg, unsigned sel, uint32_t val)
{
	uint32_t code[3] = {
		0x40840000,		/* mtc0 a0, X */
		0x03e00008,		/* jr ra */
		0x00000000,		/*  nop */
	};
	void (*fn)(uint32_t val) = (void *)code;

	code[0] |= reg << 11;
	code[0] |= sel << 0;

	flush_dcache((uintptr_t)code, sizeof(code));
	flush_icache((uintptr_t)code, sizeof(code));

	fn(val);
}

#endif /* __FW_COP0_H__ */
