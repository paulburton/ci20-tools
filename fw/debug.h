/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_DEBUG_H__
#define __FW_DEBUG_H__

#include "uart.h"
#include "util.h"

static inline void debug_init(void)
{
	if (!config_enabled(DEBUG))
		return;

	uart_init(4, 115200);
}

static inline void debug(const char *str)
{
	if (!config_enabled(DEBUG))
		return;

	uart_puts(str);
}

static inline void debug_hex(uint32_t x, unsigned digits)
{
	if (!config_enabled(DEBUG))
		return;

	uart_putx(x, digits);
}

#endif /* __FW_DEBUG_H__ */
