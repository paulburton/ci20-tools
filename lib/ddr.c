/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <errno.h>
#include <time.h>
#include "ci20.h"
#include "jz4780/cgu.h"
#include "jz4780/ddr-controller.h"
#include "jz4780/ddr-phy.h"
#include "util.h"

struct ddr_cfg {
	unsigned width;
	unsigned chip_select;
	unsigned banks;
	unsigned row;
	unsigned col;

	unsigned clk_div;
	unsigned trefi;

	unsigned bl;
	unsigned cl;

	unsigned tras;
	unsigned trp;
	unsigned trcd;
	unsigned trc;
	unsigned trrd;
	unsigned trtp;
	unsigned twtr;
	unsigned tcwl;
	unsigned tccd;

	unsigned tal;
	unsigned trl;
	unsigned twl;
	unsigned twr;
	unsigned tcke;
	unsigned txp;
	unsigned tmrd;
	unsigned trfc;
	unsigned tminsr;

	unsigned trtw;
	unsigned trdlat;
	unsigned twdlat;

	unsigned txsrd;
	unsigned tfaw;
	unsigned tmod;
	unsigned tdlllock;
	unsigned txpdll;
	unsigned txsdll;
	unsigned txs;
};

struct timings {
	unsigned rtp, wtr, wr, wl;
	unsigned ccd, ras, rcd, rl;
	unsigned cksre, rp, rrd, rc;
	unsigned rfc, extrw, rwcov, cke, minsr, xp, mrd;
	unsigned ctlupd, rtw, rdlat, wdlat;
	unsigned xsrd, faw, cfgw, cfgr;
};

static void udelay(unsigned us)
{
	struct timespec rem, req = {
		.tv_sec = us / 1000000,
		.tv_nsec = (us % 1000000) * 1000,
	};

	while (nanosleep(&req, &rem)) {
		if (errno != EINTR)
			break;

		req = rem;
	}
}

static void calc_h5tq2g83cfr(struct ddr_cfg *cfg, unsigned ps)
{
	cfg->width = 32;
	cfg->chip_select = 0x1;
	cfg->banks = 8;

	cfg->row = 15;
	cfg->col = 10;

	cfg->clk_div = 1;
	cfg->trefi = 7800;

	cfg->bl = 8;
	cfg->cl = 6;

	cfg->tras = 38;
	cfg->trp = 15;
	cfg->trcd = 15;
	cfg->trc = 53;
	cfg->trrd = DIV_ROUND_UP(max(4 * ps, 7500), 1000);
	cfg->trtp = DIV_ROUND_UP(max(4 * ps, 7500), 1000);
	cfg->twtr = DIV_ROUND_UP(max(4 * ps, 7500), 1000);
	cfg->tcke = DIV_ROUND_UP(max(3 * ps, 5625), 1000);
	cfg->txp = 3;
	cfg->tmrd = 4;
	cfg->trfc = 215;
	cfg->tminsr = 60;

	cfg->tal = 0;
	cfg->tcwl = cfg->cl - 1;

	cfg->tccd = 4;

	cfg->trl = cfg->tal + cfg->cl;
	cfg->twl = cfg->tal + cfg->tcwl;
	cfg->twr = 15;

	cfg->trtw = cfg->trl + cfg->tccd + 2 - cfg->twl;
	cfg->trdlat = cfg->trl - 2;
	cfg->twdlat = cfg->twl - 1;

	cfg->tfaw = 50;
	cfg->tdlllock = 512;
	cfg->txsdll = DIV_ROUND_UP(cfg->tdlllock * ps, 1000);
	cfg->tmod = DIV_ROUND_UP(max(12 * ps, 15000), 1000);
	cfg->txpdll = DIV_ROUND_UP(max(10 * ps, 24000), 1000);
	cfg->txs = DIV_ROUND_UP(max(5 * ps, (cfg->trfc + 10) * 1000), 1000);
	cfg->txsrd = 100;
}

static uint32_t ddr_size(unsigned cs, struct ddr_cfg *cfg)
{
	uint32_t size;

	if (!(cfg->chip_select & (1 << cs)))
		return 0;

	size = (1 << (cfg->row + cfg->col)) * (cfg->width / 8) * cfg->banks;

	if ((cs == 0) && (cfg->chip_select & (1 << 1)))
		size = min(size, 0x20000000);

	return size;

}

static int phy_init(struct ci20_dev *dev, struct ddr_cfg *cfg, struct timings *t, unsigned ps)
{
	uint32_t dcr, mr0, ptr[3], dtpr[3], pgcr, zqxcr0;
	unsigned i;
	jmp_buf env;
	int err;

	err = setjmp(env);
	if (err)
		return err;

	writej_ddrp_dtar(dev, 0x00150000, env);

	dcr = DDRP_DCR_TYPE_DDR3 << DDRP_DCR_TYPE_SHIFT;
	dcr |= (cfg->banks == 8) ? DDRP_DCR_DDR8BNK : 0;
	writej_ddrp_dcr(dev, dcr, env);

	mr0 = ((t->wr <= 8) ? (t->wr - 4) : DIV_ROUND_UP(t->wr, 2)) << DDRP_MR0_WR_SHIFT;
	mr0 |= (cfg->cl - 4) << DDRP_MR0_CL_SHIFT;
	mr0 |= ((8 - cfg->bl) / 2) << DDRP_MR0_BL_SHIFT;
	writej_ddrp_mr0(dev, mr0, env);

	writej_ddrp_mr1(dev, DDRP_MR1_DIC_RZQ7 | DDRP_MR1_RTT_DIS, env);
	writej_ddrp_odtcr(dev, 0, env);
	writej_ddrp_mr2(dev, (cfg->tcwl - 5) << DDRP_MR2_CWL_SHIFT, env);

	ptr[0] = min(DIV_ROUND_UP(50000, ps), 63) << DDRP_PTR0_PLLSRST_SHIFT;
	ptr[0] |= min(DIV_ROUND_UP(5120, ps), 0xfff) << DDRP_PTR0_PLLLOCK_SHIFT;
	ptr[0] |= 8 << DDRP_PTR0_ITMSRST_SHIFT;
	writej_ddrp_ptr0(dev, ptr[0], env);

	ptr[1] = min(DIV_ROUND_UP(500000000, ps), 0x7ffff) << DDRP_PTR1_DINIT0_SHIFT;
	ptr[1] |= min(DIV_ROUND_UP(max((cfg->trfc + 10) * 1000, 5 * ps), ps), 0xff) << DDRP_PTR1_DINIT1_SHIFT;
	writej_ddrp_ptr1(dev, ptr[1], env);

	ptr[2] = min(DIV_ROUND_UP(200000000, ps), 0x1ffff) << DDRP_PTR2_DINIT2_SHIFT;
	ptr[2] |= min(512, 0x3ff) << DDRP_PTR2_DINIT3_SHIFT;
	writej_ddrp_ptr2(dev, ptr[2], env);

	dtpr[0] = cfg->tmrd - 4;
	dtpr[0] |= max(t->rtp, 2) << 2;
	dtpr[0] |= t->wtr << 5;
	dtpr[0] |= max(t->rp, 2) << 8;
	dtpr[0] |= max(t->rcd, 2) << 12;
	dtpr[0] |= max(t->ras, 2) << 16;
	dtpr[0] |= t->rrd << 21;
	dtpr[0] |= max(t->rc, 2) << 25;
	dtpr[0] |= (cfg->tmrd > 4) ? (1 << 31) : 0;
	writej_ddrp_dtpr0(dev, dtpr[0], env);

	dtpr[1] = bound(2, DIV_ROUND_UP(cfg->tfaw * 1000, ps), 31) << 3;
	dtpr[1] |= min(DIV_ROUND_UP(cfg->tmod * 1000, ps) - 12, 3) << 9;
	dtpr[1] |= 1 << 11;
	dtpr[1] |= bound(1, DIV_ROUND_UP(cfg->trfc * 1000, ps), 255) << 16;
	writej_ddrp_dtpr1(dev, dtpr[1], env);

	dtpr[2] = bound(2, DIV_ROUND_UP(max(cfg->txs, cfg->txsdll) * 1000, ps), 1023) << 0;
	dtpr[2] |= bound(2, DIV_ROUND_UP(max(cfg->txp, cfg->txpdll) * 1000, ps), 31) << 10;
	dtpr[2] |= bound(2, cfg->tcke, 15) << 15;
	dtpr[2] |= bound(2, cfg->tdlllock, 1023) << 19;
	writej_ddrp_dtpr2(dev, dtpr[2], env);

	pgcr = DDRP_PGCR_DQSCFG;
	pgcr |= 7 << DDRP_PGCR_CKEN_SHIFT;
	pgcr |= 2 << DDRP_PGCR_CKDV_SHIFT;
	pgcr |= cfg->chip_select << DDRP_PGCR_RANKEN_SHIFT;
	pgcr |= DDRP_PGCR_ZCKSEL_32;
	pgcr |= DDRP_PGCR_PDDISDX;
	writej_ddrp_pgcr(dev, pgcr, env);

	for (i = 0; i < 8; i++)
		clearj_ddrp_dxgcr(dev, i, 0x3 << 9, env);

	while (readj_ddrp_pgsr(dev, env) != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE)) {
		if (readj_ddrp_pgsr(dev, env) == 0x1f)
			break;
	}

	writej_ddrp_pir(dev, DDRP_PIR_INIT | DDRP_PIR_DRAMINT | DDRP_PIR_DRAMRST, env);

	while (readj_ddrp_pgsr(dev, env) != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE | DDRP_PGSR_DIDONE)) {
		if (readj_ddrp_pgsr(dev, env) == 0x1f)
			break;
	}

	writej_ddrp_pir(dev, DDRP_PIR_INIT | DDRP_PIR_QSTRN, env);

	while (readj_ddrp_pgsr(dev, env) != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE | DDRP_PGSR_DIDONE | DDRP_PGSR_DTDONE)) {
		if (readj_ddrp_pgsr(dev, env) == 0x1f)
			break;
	}

	zqxcr0 = readj_ddrp_zqxcr0(dev, 0, env);
	zqxcr0 &= ~0x3ff;
	zqxcr0 |= 0xe << 0;
	zqxcr0 |= 0xe << 5;
	zqxcr0 |= 1 << 28;
	writej_ddrp_zqxcr0(dev, 0, zqxcr0, env);

	return 0;
}

static void remap_swap(struct ci20_dev *dev, unsigned a, unsigned b, jmp_buf env)
{
	unsigned a_bit = (a % 4) * 8;
	unsigned b_bit = (b % 4) * 8;
	unsigned a_mask = 0x1f << a_bit;
	unsigned b_mask = 0x1f << b_bit;
	uint32_t remmap[2], tmp[2];

	remmap[0] = readj_ddrc_dremap(dev, a / 4 + 1, env);
	remmap[1] = readj_ddrc_dremap(dev, b / 4 + 1, env);

	tmp[0] = (remmap[0] & a_mask) >> a_bit;
	tmp[1] = (remmap[1] & b_mask) >> b_bit;

	remmap[0] &= ~a_mask;
	remmap[1] &= ~b_mask;

	writej_ddrc_dremap(dev, a / 4 + 1, remmap[0] | (tmp[1] << a_bit), env);
	writej_ddrc_dremap(dev, b / 4 + 1, remmap[1] | (tmp[0] << b_bit), env);
}

static void mem_remap(struct ci20_dev *dev, struct ddr_cfg *cfg, jmp_buf env)
{
	unsigned start, num;

	start = cfg->row + cfg->col + (cfg->width / 8) / 2;
	start -= 12;

#if 0
	if (cfg->banks == 8)
		num = 3;
	else
		num = 2;
#else
	num = ffs(cfg->banks) - 1;
#endif

	if (cfg->chip_select == 0x3)
		num++;

	for (; num > 0; num--)
		remap_swap(dev, 0 + num - 1, start + num - 1, env);
}

int ci20_ddr_init(struct ci20_dev *dev, unsigned hz)
{
	struct ddr_cfg cfg;
	struct timings t;
	unsigned ps;
	uint32_t timing1, timing2, timing3, timing4, timing5, timing6, dcfg, drefcnt;
	uint32_t mem_size[2], mem_base[2], mem_mask[2];
	jmp_buf env;
	int err;

	err = setjmp(env);
	if (err)
		return err;

	/* reset DDR phy PLL */
	writej_cgu_drcg(dev, CGU_DRCG_DLLRESET | CGU_DRCG_CFGCLKEN, env);
	udelay(400000);
	writej_cgu_drcg(dev, CGU_DRCG_CFGCLKEN, env);
	udelay(400000);

	/* calculate DDR parameters */
	ps = 1000000000 / (hz / 1000);
	calc_h5tq2g83cfr(&cfg, ps);

	/* enter reset */
	writej_ddrc_dctrl(dev, DDRC_DCTRL_DFI_RST | DDRC_DCTRL_DLL_RST |
			  DDRC_DCTRL_CTL_RST | DDRC_DCTRL_CFG_RST, env);

	/* calculate timing1 values */
	t.rtp = bound(1, DIV_ROUND_UP(cfg.trtp * 1000, ps), 6);
	t.wtr = bound(1, DIV_ROUND_UP(cfg.twtr * 1000, ps), 6);
	t.wr = bound(5, DIV_ROUND_UP(cfg.twr * 1000, ps), 12);
	t.wl = bound(1, cfg.twl, 63);

	/* calculate timing2 values */
	t.ccd = bound(1, cfg.tccd, 63);
	t.ras = bound(1, DIV_ROUND_UP(cfg.tras * 1000, ps), 31);
	t.rcd = bound(1, DIV_ROUND_UP(cfg.trcd * 1000, ps), 11);
	t.rl = bound(1, cfg.trl, 63);

	/* calculate timing3 values */
	t.cksre = 0x7;
	t.rp = bound(1, DIV_ROUND_UP(cfg.trp * 1000, ps), 11);
	t.rrd = bound(1, DIV_ROUND_UP(cfg.trrd * 1000, ps), 8);
	t.rc = bound(1, DIV_ROUND_UP(cfg.trc * 1000, ps), 42);

	/* calculate timing4 values */
	t.rfc = bound(1, (DIV_ROUND_UP(cfg.trfc * 1000, ps) - 1) / 2, 63);
	t.extrw = 0;
	t.rwcov = 1;
	t.cke = DIV_ROUND_UP(cfg.tcke * 1000, ps);
	t.minsr = DIV_ROUND_UP(bound(9, cfg.tminsr, 129) - 9, 8);
	t.xp = cfg.txp;
	t.mrd = cfg.tmrd - 1;

	/* calculate timing5 values */
	t.ctlupd = 0;
	t.rtw = bound(1, cfg.trtw, 63);
	t.rdlat = min_t(unsigned, cfg.trdlat, 63);
	t.wdlat = min_t(unsigned, cfg.twdlat, 63);

	/* calculate timing6 values */
	t.xsrd = bound(1, cfg.txsrd / 4, 255);
	t.faw = bound(1, DIV_ROUND_UP(cfg.tfaw * 1000, ps), 31);
	t.cfgw = 2;
	t.cfgr = 2;

	writej_ddrc_dctrl(dev, 0, env);

	err = phy_init(dev, &cfg, &t, ps);
	if (err)
		return err;

	writej_ddrc_dctrl(dev, DDRC_DCTRL_CKE | DDRC_DCTRL_ALH, env);
	writej_ddrc_dctrl(dev, 0, env);

	dcfg = DDRC_DCFG_IMBA | DDRC_DCFG_MISPE;
	dcfg |= DDRC_DCFG_TYPE_DDR3 << DDRC_DCFG_TYPE_SHIFT;
	dcfg |= (cfg.width == 32) ? 1 : 0;
	dcfg |= (cfg.banks == 8) ? (1 << 1) : 0;
	dcfg |= (0x8 | bound(0, cfg.cl - 1, 4)) << 2;
	dcfg |= cfg.chip_select << 6;
	dcfg |= (cfg.col - 8) << 8;
	dcfg |= (cfg.row - 12) << 11;
	dcfg |= (cfg.bl > 4) ? (1 << 21) : 0;
	dcfg |= (cfg.banks == 8) ? (1 << 23) : 0;
	dcfg |= (cfg.col - 8) << 24;
	dcfg |= (cfg.row - 12) << 27;
	writej_ddrc_dcfg(dev, dcfg, env);

	timing1 = t.rtp << DDRC_DTIMING1_TRTP_SHIFT;
	timing1 |= (cfg.twl + cfg.bl / 2 + t.wtr) << DDRC_DTIMING1_TWTR_SHIFT;
	timing1 |= t.wr << DDRC_DTIMING1_TWR_SHIFT;
	timing1 |= t.wl << DDRC_DTIMING1_TWL_SHIFT;
	writej_ddrc_dtiming1(dev, timing1, env);

	timing2 = t.ccd << DDRC_DTIMING2_TCCD_SHIFT;
	timing2 |= t.ras << DDRC_DTIMING2_TRAS_SHIFT;
	timing2 |= t.rcd << DDRC_DTIMING2_TRCD_SHIFT;
	timing2 |= t.rl << DDRC_DTIMING2_TRL_SHIFT;
	writej_ddrc_dtiming2(dev, timing2, env);

	timing3 = 0x4 << DDRC_DTIMING3_ONUM_SHIFT;
	timing3 |= t.cksre << DDRC_DTIMING3_TCKSRE_SHIFT;
	timing3 |= t.rp << DDRC_DTIMING3_TRP_SHIFT;
	timing3 |= t.rrd << DDRC_DTIMING3_TRRD_SHIFT;
	timing3 |= t.rc << DDRC_DTIMING3_TRC_SHIFT;
	writej_ddrc_dtiming3(dev, timing3, env);

	timing4 = t.rfc << DDRC_DTIMING4_TRFC_SHIFT;
	timing4 |= t.extrw << DDRC_DTIMING4_TEXTRW_SHIFT;
	timing4 |= t.rwcov << DDRC_DTIMING4_TRWCOV_SHIFT;
	timing4 |= t.cke << DDRC_DTIMING4_TCKE_SHIFT;
	timing4 |= t.minsr << DDRC_DTIMING4_TMINSR_SHIFT;
	timing4 |= t.xp << DDRC_DTIMING4_TXP_SHIFT;
	timing4 |= t.mrd << DDRC_DTIMING4_TMRD_SHIFT;
	writej_ddrc_dtiming4(dev, timing4, env);

	timing5 = t.ctlupd << DDRC_DTIMING5_TCTLUPD_SHIFT;
	timing5 |= t.rtw << DDRC_DTIMING5_TRTW_SHIFT;
	timing5 |= t.rdlat << DDRC_DTIMING5_TRDLAT_SHIFT;
	timing5 |= t.wdlat << DDRC_DTIMING5_TWDLAT_SHIFT;
	writej_ddrc_dtiming5(dev, timing5, env);

	timing6 = t.xsrd << DDRC_DTIMING6_TXSRD_SHIFT;
	timing6 |= t.faw << DDRC_DTIMING6_TFAW_SHIFT;
	timing6 |= t.cfgw << DDRC_DTIMING6_TCFGW_SHIFT;
	timing6 |= t.cfgr << DDRC_DTIMING6_TCFGR_SHIFT;
	writej_ddrc_dtiming6(dev, timing6, env);

	mem_size[0] = ddr_size(0, &cfg);
	mem_size[1] = ddr_size(1, &cfg);

	if (!mem_size[1] && mem_size[0] > 0x20000000) {
		mem_base[0] = 0;
		mem_mask[0] = ~(((mem_size[0] * 2) >> 24) - 1) & DDRC_DMMAPX_MASK;
	} else {
		mem_base[0] = (DDRP_MEM_PHY_BASE >> 24) & 0xff;
		mem_mask[0] = ~((mem_size[0] >> 24) - 1) & DDRC_DMMAPX_MASK;
	}

	if (mem_size[1]) {
		mem_base[1] = ((DDRP_MEM_PHY_BASE + mem_size[0]) >> 24) & 0xff;
		mem_mask[1] = ~((mem_size[1] >> 24) - 1) & DDRC_DMMAPX_MASK;
	} else {
		mem_base[1] = 0xff;
		mem_mask[1] = 0;
	}

	writej_ddrc_dmmap0(dev, mem_base[0] << DDRC_DMMAPX_BASE_SHIFT | mem_mask[0], env);
	writej_ddrc_dmmap1(dev, mem_base[1] << DDRC_DMMAPX_BASE_SHIFT | mem_mask[1], env);
	writej_ddrc_dctrl(dev, DDRC_DCTRL_CKE | DDRC_DCTRL_ALH, env);

	drefcnt = DDRC_DREFCNT_REF_EN;
	drefcnt |= cfg.clk_div << DDRC_DREFCNT_CLK_DIV_SHIFT;
	drefcnt |= bound(1, (cfg.trefi / DIV_ROUND_UP(1000000000, hz)) / (16 * (1 << cfg.clk_div)) - 1, 0xff) << DDRC_DREFCNT_CON_SHIFT;
	writej_ddrc_drefcnt(dev, drefcnt, env);

	writej_ddrc_dctrl(dev, 1 << 15 | 4 << 12 | 1 << 11 | 1 << 8 | 0 << 6 | 1 << 4 | 1 << 3 | 1 << 2 | 1 << 1, env);
	mem_remap(dev, &cfg, env);
	clearj_ddrc_dstatus(dev, DDRC_DSTATUS_MISS, env);

	return 0;
}
