/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_CPM_H__
#define __FW_CPM_H__

#include "io.h"

#define CPM_BASE 0xb0000000

#define CPM_GEN_ACCESSORS(offset, name)			\
static inline uint32_t read_##name(void)		\
{							\
	return readl((void *)CPM_BASE + (offset));	\
}							\
							\
static inline void write_##name(uint32_t val)		\
{							\
	writel(val, (void *)CPM_BASE + (offset));	\
}

CPM_GEN_ACCESSORS(0x20, clkgr0)
CPM_GEN_ACCESSORS(0x28, clkgr1)

#undef CPM_GEN_ACCESSORS

#define CPM_CLKGR1_UART4	(1 << 10)

#endif /* __FW_CPM_H__ */
