/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <errno.h>
#include <setjmp.h>

#include "ci20.h"
#include "jz4780/cgu.h"

static uint32_t readj_cgu_pll(struct ci20_dev *dev, enum ci20_pll pll, jmp_buf env)
{
	switch (pll) {
	case PLL_A:
		return readj_cgu_cpapcr(dev, env);

	case PLL_M:
		return readj_cgu_cpmpcr(dev, env);

	case PLL_E:
		return readj_cgu_cpepcr(dev, env);

	case PLL_V:
		return readj_cgu_cpvpcr(dev, env);

	default:
		longjmp(env, -EINVAL);
	}
}

static void writej_cgu_pll(struct ci20_dev *dev, enum ci20_pll pll, uint32_t val, jmp_buf env)
{
	switch (pll) {
	case PLL_A:
		writej_cgu_cpapcr(dev, val, env);
		break;

	case PLL_M:
		writej_cgu_cpmpcr(dev, val, env);
		break;

	case PLL_E:
		writej_cgu_cpepcr(dev, val, env);
		break;

	case PLL_V:
		writej_cgu_cpvpcr(dev, val, env);
		break;

	default:
		longjmp(env, -EINVAL);
	}
}

int ci20_pll_init(struct ci20_dev *dev, enum ci20_pll pll, unsigned m, unsigned n, unsigned od)
{
	uint32_t pll_reg;
	jmp_buf env;
	int err;

	err = setjmp(env);
	if (err)
		return err;

	pll_reg = readj_cgu_pll(dev, pll, env);
	pll_reg &= ~(CGU_CPXPCR_M | CGU_CPXPCR_N | CGU_CPXPCR_OD);
	pll_reg |= (m - 1) << CGU_CPXPCR_M_SHIFT;
	pll_reg |= (n - 1) << CGU_CPXPCR_N_SHIFT;
	pll_reg |= (od - 1) << CGU_CPXPCR_OD_SHIFT;
	pll_reg |= CGU_CPXPCR_EN;
	writej_cgu_pll(dev, pll, pll_reg, env);

	do {
		pll_reg = readj_cgu_pll(dev, pll, env);
	} while (!(pll_reg & CGU_CPXPCR_ON));

	return 0;
}

int ci20_mux_cpu_clk(struct ci20_dev *dev, enum ci20_cpu_clk clk)
{
	uint32_t ctrl, status;
	jmp_buf env;
	int err;

	err = setjmp(env);
	if (err)
		return err;

	/* read the current clock mux select bits */
	ctrl = readj_cgu_cpccr(dev, env);
	ctrl &= CGU_CPCCR_SEL_SRC | CGU_CPCCR_SEL_CPLL | CGU_CPCCR_SEL_H0PLL | CGU_CPCCR_SEL_H2PLL;

	/* set the clock divides */
	ctrl |= (12 - 1) << CGU_CPCCR_PDIV_SHIFT;
	ctrl |= (6 - 1) << CGU_CPCCR_H2DIV_SHIFT;
	ctrl |= (3 - 1) << CGU_CPCCR_H0DIV_SHIFT;
	ctrl |= (2 - 1) << CGU_CPCCR_L2CDIV_SHIFT;
	ctrl |= (1 - 1) << CGU_CPCCR_CDIV_SHIFT;
	ctrl |= CGU_CPCCR_CE_CPU | CGU_CPCCR_CE_AHB0 | CGU_CPCCR_CE_AHB2;
	writej_cgu_cpccr(dev, ctrl, env);

	/* wait for clocks to stabilise */
	do {
		status = readj_cgu_cpcsr(dev, env);
	} while (status & (CGU_CPCSR_CDIV_BUSY | CGU_CPCSR_H0DIV_BUSY | CGU_CPCSR_H2DIV_BUSY));

	/* set mux select bits */
	ctrl &= ~(CGU_CPCCR_SEL_SRC | CGU_CPCCR_SEL_CPLL | CGU_CPCCR_SEL_H0PLL | CGU_CPCCR_SEL_H2PLL);
	ctrl |= 0x2 << CGU_CPCCR_SEL_SRC_SHIFT;
	ctrl |= (unsigned)clk << CGU_CPCCR_SEL_CPLL_SHIFT;
	ctrl |= 0x2 << CGU_CPCCR_SEL_H0PLL_SHIFT;
	ctrl |= 0x2 << CGU_CPCCR_SEL_H2PLL_SHIFT;
	writej_cgu_cpccr(dev, ctrl, env);

	return 0;
}

int ci20_mux_ddr_clk(struct ci20_dev *dev, enum ci20_ddr_clk clk, unsigned div)
{
	uint32_t ctrl, clkgr0;
	jmp_buf env;
	int err;

	err = setjmp(env);
	if (err)
		return err;

	/* set up the divider & mux */
	ctrl = (unsigned)clk << CGU_DDRCDR_DCS_SHIFT;
	ctrl |= CGU_DDRCDR_CE | ((div - 1) << CGU_DDRCDR_DIV_SHIFT);
	writej_cgu_ddrcdr(dev, ctrl, env);

	/* wait for the divider to stabilise */
	do {
		ctrl = readj_cgu_ddrcdr(dev, env);
	} while (ctrl & CGU_DDRCDR_BUSY);

	/* ungate the DDR */
	clkgr0 = readj_cgu_clkgr0(dev, env);
	clkgr0 &= ~CGU_CLKGR0_DDR0;
	writej_cgu_clkgr0(dev, clkgr0, env);

	return 0;
}
