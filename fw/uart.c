/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>

#include "cpm.h"
#include "gpio.h"
#include "io.h"
#include "uart.h"

static void *uart_base;
static unsigned uart_baud;
static const unsigned uart_clk = 48000000;

#define UART_GEN_ACCESSORS(idx, name)			\
static inline uint8_t read_##name(void)			\
{							\
	return readb(uart_base + (idx * 4));		\
}							\
							\
static inline void write_##name(uint8_t val)		\
{							\
	writeb(val, uart_base + (idx * 4));		\
}

UART_GEN_ACCESSORS(0, rbr)
UART_GEN_ACCESSORS(0, dll)
UART_GEN_ACCESSORS(0, thr)
UART_GEN_ACCESSORS(1, ier)
UART_GEN_ACCESSORS(1, dlm)
UART_GEN_ACCESSORS(2, fcr)
UART_GEN_ACCESSORS(3, lcr)
UART_GEN_ACCESSORS(4, mcr)
UART_GEN_ACCESSORS(5, lsr)
UART_GEN_ACCESSORS(6, msr)
UART_GEN_ACCESSORS(7, spr)

#undef UART_GEN_ACCESSORS

#define FCR_FIFO_EN	(1 << 0)
#define FCR_RXSR	(1 << 1)
#define FCR_TXSR	(1 << 2)
#define FCR_UME		(1 << 4)

#define LCR_8N1		(0x3 << 0)
#define LCR_BKSE	(1 << 7)

#define MCR_DTR		(1 << 0)
#define MCR_RTS		(1 << 1)

#define LSR_THRE	(1 << 5)
#define LSR_TEMT	(1 << 6)

void uart_init(unsigned uart, unsigned baud)
{
	unsigned divisor;

	uart_base = (void *)(0xb0030000 + (uart * 0x1000));
	uart_baud = baud;
	divisor = (uart_clk + (uart_baud * (16 / 2))) / (16 * uart_baud);

	/* mux pins */
	write_pxintc(2, 0x100400);
	write_pxmskc(2, 0x100400);
	write_pxpat1s(2, 0x100400);
	write_pxpat0c(2, 0x100400);
	write_pxpenc(2, 0x100400);

	/* ungate UART clock */
	write_clkgr1(read_clkgr1() & ~CPM_CLKGR1_UART4);

	while (!(read_lsr() & LSR_TEMT));
	write_ier(0);
	write_lcr(LCR_BKSE | LCR_8N1);
	write_dll(0);
	write_dlm(0);
	write_lcr(LCR_8N1);
	write_mcr(MCR_DTR | MCR_RTS);
	write_fcr(FCR_FIFO_EN | FCR_RXSR | FCR_TXSR | FCR_UME);
	write_lcr(LCR_BKSE | LCR_8N1);
	write_dll(divisor & 0xff);
	write_dlm((divisor >> 8) & 0xff);
	write_lcr(LCR_8N1);
}

void uart_putc(char c)
{
	while (!(read_lsr() & LSR_THRE));
	write_thr(c);
}

void uart_puts(const char *s)
{
	while (*s)
		uart_putc(*s++);
}

static char hex_char(uint8_t x)
{
	if (x > 9)
		return 'a' + x - 10;

	return '0' + x;
}

void uart_putx(uint32_t x, unsigned digits)
{
	char buf[9];
	int idx = 7;

	buf[8] = 0;

	do {
		buf[idx--] = hex_char(x & 0xf);
		x >>= 4;
	} while (x || ((7 - idx) < digits));

	uart_puts(&buf[idx + 1]);
}
