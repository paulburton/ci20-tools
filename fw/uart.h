/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_UART_H__
#define __FW_UART_H__

#include <stdint.h>

extern void uart_init(unsigned uart, unsigned baud);
extern void uart_putc(char c);
extern void uart_puts(const char *s);
extern void uart_putx(uint32_t x, unsigned digits);

#endif /* __FW_UART_H__ */
