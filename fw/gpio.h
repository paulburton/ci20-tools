/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_GPIO_H__
#define __FW_GPIO_H__

#include "io.h"

#define GPIO_BASE 0xb0010000

#define GPIO_GEN_ACCESSORS(offset, name)				\
static inline uint32_t read_##name(unsigned port)			\
{									\
	return readl((void *)GPIO_BASE + (port * 0x100) + (offset));	\
}									\
									\
static inline void write_##name(unsigned port, uint32_t value)		\
{									\
	writel(value, (void *)GPIO_BASE + (port * 0x100) + (offset));	\
}

GPIO_GEN_ACCESSORS(0x00, pxpin)
GPIO_GEN_ACCESSORS(0x10, pxint)
GPIO_GEN_ACCESSORS(0x14, pxints)
GPIO_GEN_ACCESSORS(0x18, pxintc)
GPIO_GEN_ACCESSORS(0x20, pxmsk)
GPIO_GEN_ACCESSORS(0x24, pxmsks)
GPIO_GEN_ACCESSORS(0x28, pxmskc)
GPIO_GEN_ACCESSORS(0x30, pxpat1)
GPIO_GEN_ACCESSORS(0x34, pxpat1s)
GPIO_GEN_ACCESSORS(0x38, pxpat1c)
GPIO_GEN_ACCESSORS(0x40, pxpat0)
GPIO_GEN_ACCESSORS(0x44, pxpat0s)
GPIO_GEN_ACCESSORS(0x48, pxpat0c)
GPIO_GEN_ACCESSORS(0x70, pxpen)
GPIO_GEN_ACCESSORS(0x74, pxpens)
GPIO_GEN_ACCESSORS(0x78, pxpenc)

#undef GPIO_GEN_ACCESSORS

#endif /* __FW_GPIO_H__ */
