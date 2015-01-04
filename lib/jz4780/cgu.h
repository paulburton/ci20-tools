/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __COMMON_CGU_H__
#define __COMMON_CGU_H__

#include "reg-accessors.h"

#define CGU_BASE 0xb0000000

#define CGU_GEN_ACCESSORS(offset, name) \
	GEN_ACCESSORS(CGU_BASE + (offset), cgu_##name)

CGU_GEN_ACCESSORS(0x00, cpccr)
CGU_GEN_ACCESSORS(0x0c, cppcr)
CGU_GEN_ACCESSORS(0x10, cpapcr)
CGU_GEN_ACCESSORS(0x14, cpmpcr)
CGU_GEN_ACCESSORS(0x18, cpepcr)
CGU_GEN_ACCESSORS(0x1c, cpvpcr)
CGU_GEN_ACCESSORS(0x20, clkgr0)
CGU_GEN_ACCESSORS(0x2c, ddrcdr)
CGU_GEN_ACCESSORS(0x30, vpucdr)
CGU_GEN_ACCESSORS(0xd0, drcg)
CGU_GEN_ACCESSORS(0xd4, cpcsr)

#undef CGU_GEN_ACCESSORS

/* CPCCR */
#define CGU_CPCCR_SEL_SRC		(0x3 << 30)
#define CGU_CPCCR_SEL_SRC_SHIFT		30
#define CGU_CPCCR_SEL_CPLL		(0x3 << 28)
#define CGU_CPCCR_SEL_CPLL_SHIFT	28
#define CGU_CPCCR_SEL_H0PLL		(0x3 << 26)
#define CGU_CPCCR_SEL_H0PLL_SHIFT	26
#define CGU_CPCCR_SEL_H2PLL		(0x3 << 24)
#define CGU_CPCCR_SEL_H2PLL_SHIFT	24
#define CGU_CPCCR_CE_CPU		(1 << 22)
#define CGU_CPCCR_CE_AHB0		(1 << 21)
#define CGU_CPCCR_CE_AHB2		(1 << 20)
#define CGU_CPCCR_PDIV			(0xf << 16)
#define CGU_CPCCR_PDIV_SHIFT		16
#define CGU_CPCCR_H2DIV			(0xf << 12)
#define CGU_CPCCR_H2DIV_SHIFT		12
#define CGU_CPCCR_H0DIV			(0xf << 8)
#define CGU_CPCCR_H0DIV_SHIFT		8
#define CGU_CPCCR_L2CDIV		(0xf << 4)
#define CGU_CPCCR_L2CDIV_SHIFT		4
#define CGU_CPCCR_CDIV			(0xf << 0)
#define CGU_CPCCR_CDIV_SHIFT		0

/* DDRCDR */
#define CGU_DDRCDR_DCS			(0x3 << 30)
#define CGU_DDRCDR_DCS_SHIFT		30
#define CGU_DDRCDR_CE			(1 << 29)
#define CGU_DDRCDR_BUSY			(1 << 28)
#define CGU_DDRCDR_STOP			(1 << 27)
#define CGU_DDRCDR_DIV			(0xf << 0)
#define CGU_DDRCDR_DIV_SHIFT		0

/* CLKGR0 */
#define CGU_CLKGR0_DDR1			(1 << 31)
#define CGU_CLKGR0_DDR0			(1 << 30)

/* CPxPCR */
#define CGU_CPXPCR_M			(0x1fff << 19)
#define CGU_CPXPCR_M_SHIFT		19
#define CGU_CPXPCR_N			(0x3f << 13)
#define CGU_CPXPCR_N_SHIFT		13
#define CGU_CPXPCR_OD			(0xf << 9)
#define CGU_CPXPCR_OD_SHIFT		9
#define CGU_CPXPCR_ON			(1 << 4)
#define CGU_CPXPCR_EN			(1 << 0)

/* DRCG */
#define CGU_DRCG_DLLRESET		(1 << 1)
#define CGU_DRCG_CFGCLKEN		(1 << 0)

/* CPCSR */
#define CGU_CPCSR_H2DIV_BUSY		(1 << 2)
#define CGU_CPCSR_H0DIV_BUSY		(1 << 1)
#define CGU_CPCSR_CDIV_BUSY		(1 << 0)

#endif /* __COMMON_CGU_H__ */
