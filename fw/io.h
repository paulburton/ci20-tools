/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_IO_H__
#define __FW_IO_H__

#include <stdint.h>

static inline uint8_t readb(const void *addr)
{
	return *((volatile uint8_t *)addr);
}

static inline uint16_t readw(const void *addr)
{
	return *((volatile uint16_t *)addr);
}

static inline uint32_t readl(const void *addr)
{
	return *((volatile uint32_t *)addr);
}

static inline void writeb(uint8_t val, const void *addr)
{
	*((volatile uint8_t *)addr) = val;
}

static inline void writew(uint16_t val, const void *addr)
{
	*((volatile uint16_t *)addr) = val;
}

static inline void writel(uint32_t val, const void *addr)
{
	*((volatile uint32_t *)addr) = val;
}

#endif /* __FW_IO_H__ */
