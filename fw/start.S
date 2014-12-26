/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "regs.h"

.section .text.start
.set noreorder

.extern __bss_start
.extern __bss_end
.extern main

.globl start
start:
	/* clear BSS */
	la	t0, __bss_start
	la	t1, __bss_end - 4
1:	sw	zero, 0(t0)
	blt	t0, t1, 1b
	 addiu	t0, t0, 4

	/* setup stack at the end of SRAM */
	li	sp, 0xf4000000 + (16 << 10)

	/* branch to C */
	b	main
