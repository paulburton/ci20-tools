/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JZ4780_DDR_PHY_H__
#define __JZ4780_DDR_PHY_H__

#include "reg-accessors.h"

#define DDRP_BASE 0xb3011000

#define DDRP_GEN_ACCESSORS(offset, name) \
	GEN_ACCESSORS(DDRP_BASE + (offset), ddrp_##name)

#define DDRP_GEN_IDX_ACCESSORS(offset, stride, name) \
	GEN_IDX_ACCESSORS(DDRP_BASE + (offset), stride, ddrp_##name)

DDRP_GEN_ACCESSORS(0x04, pir)
DDRP_GEN_ACCESSORS(0x08, pgcr)
DDRP_GEN_ACCESSORS(0x0c, pgsr)
DDRP_GEN_ACCESSORS(0x18, ptr0)
DDRP_GEN_ACCESSORS(0x1c, ptr1)
DDRP_GEN_ACCESSORS(0x20, ptr2)
DDRP_GEN_ACCESSORS(0x24, aciocr)
DDRP_GEN_ACCESSORS(0x28, dxccr)
DDRP_GEN_ACCESSORS(0x2c, dsgcr)
DDRP_GEN_ACCESSORS(0x30, dcr)
DDRP_GEN_ACCESSORS(0x34, dtpr0)
DDRP_GEN_ACCESSORS(0x38, dtpr1)
DDRP_GEN_ACCESSORS(0x3c, dtpr2)
DDRP_GEN_ACCESSORS(0x40, mr0)
DDRP_GEN_ACCESSORS(0x44, mr1)
DDRP_GEN_ACCESSORS(0x48, mr2)
DDRP_GEN_ACCESSORS(0x4c, mr3)
DDRP_GEN_ACCESSORS(0x50, odtcr)
DDRP_GEN_ACCESSORS(0x54, dtar)
DDRP_GEN_ACCESSORS(0x58, dtdr0)
DDRP_GEN_ACCESSORS(0x5c, dtdr1)
DDRP_GEN_ACCESSORS(0xc0, dcuar)
DDRP_GEN_ACCESSORS(0xc4, dcudr)
DDRP_GEN_ACCESSORS(0xc8, dcurr)
DDRP_GEN_ACCESSORS(0xcc, dculr)
DDRP_GEN_ACCESSORS(0xd0, dcugcr)
DDRP_GEN_ACCESSORS(0xd4, dcutpr)
DDRP_GEN_ACCESSORS(0xd8, dcusr0)
DDRP_GEN_ACCESSORS(0xdc, dcusr1)
DDRP_GEN_IDX_ACCESSORS(0x180, 0x10, zqxcr0)
DDRP_GEN_IDX_ACCESSORS(0x184, 0x10, zqxcr1)
DDRP_GEN_IDX_ACCESSORS(0x188, 0x10, zqxsr0)
DDRP_GEN_IDX_ACCESSORS(0x18c, 0x10, zqxsr1)
DDRP_GEN_IDX_ACCESSORS(0x1c0, 0x40, dxgcr)
DDRP_GEN_IDX_ACCESSORS(0x1c4, 0x40, dxgsr0)
DDRP_GEN_IDX_ACCESSORS(0x1c8, 0x40, dxgsr1)
DDRP_GEN_IDX_ACCESSORS(0x1d4, 0x40, dxdqstr)

#undef DDRP_GEN_ACCESSORS
#undef DDRP_GEN_IDX_ACCESSORS

/* PIR: PHY Initialization Register */
#define DDRP_PIR_DLLBYP			(1 << 17)
#define DDRP_PIR_EYETRN			(1 << 8)
#define DDRP_PIR_QSTRN			(1 << 7)
#define DDRP_PIR_DRAMINT		(1 << 6)
#define DDRP_PIR_DRAMRST		(1 << 5)
#define DDRP_PIR_ITMSRST		(1 << 4)
#define DDRP_PIR_ZCAL			(1 << 3)
#define DDRP_PIR_DLLLOCK		(1 << 2)
#define DDRP_PIR_DLLSRST		(1 << 1)
#define DDRP_PIR_INIT			(1 << 0)

/* PGCR: PHY General Configuration Register */
#define DDRP_PGCR_PDDISDX		(1 << 24)
#define DDRP_PGCR_ZCKSEL_32		(2 << 22)
#define DDRP_PGCR_RANKEN		(0x3 << 18)
#define DDRP_PGCR_RANKEN_SHIFT		18
#define DDRP_PGCR_CKINV			(1 << 14)
#define DDRP_PGCR_CKDV_SHIFT		12
#define DDRP_PGCR_CKEN_SHIFT		9
#define DDRP_PGCR_DTOSEL_SHIFT		5
#define DDRP_PGCR_DFTLMT_SHIFT		3
#define DDRP_PGCR_DFTCMP		(1 << 2)
#define DDRP_PGCR_DQSCFG		(1 << 1)
#define DDRP_PGCR_ITMDMD		(1 << 0)

/* PGSR: PHY General Status Register */
#define DDRP_PGSR_DFTEERR		(1 << 7)
#define DDRP_PGSR_DTIERR		(1 << 6)
#define DDRP_PGSR_DTERR			(1 << 5)
#define DDRP_PGSR_DTDONE		(1 << 4)
#define DDRP_PGSR_DIDONE		(1 << 3)
#define DDRP_PGSR_ZCDONE		(1 << 2)
#define DDRP_PGSR_DLDONE		(1 << 1)
#define DDRP_PGSR_IDONE			(1 << 0)

/* PTR0: PHY Timing Register 0 */
#define DDRP_PTR0_ITMSRST_SHIFT		18
#define DDRP_PTR0_PLLLOCK		(0xfff << 6)
#define DDRP_PTR0_PLLLOCK_SHIFT		6
#define DDRP_PTR0_PLLSRST		(0x3f << 0)
#define DDRP_PTR0_PLLSRST_SHIFT		0

/* PTR1: PHY Timing Register 1 */
#define DDRP_PTR1_DINIT1		(0xff << 19)
#define DDRP_PTR1_DINIT1_SHIFT		19
#define DDRP_PTR1_DINIT0		(0x7ffff << 0)
#define DDRP_PTR1_DINIT0_SHIFT		0

/* PTR2: PHY Timing Register 2 */
#define DDRP_PTR2_DINIT3		(0x3ff << 17)
#define DDRP_PTR2_DINIT3_SHIFT		17
#define DDRP_PTR2_DINIT2		(0x1ffff << 0)
#define DDRP_PTR2_DINIT2_SHIFT		0

/* DCR: DRAM Configuration Register */
#define DDRP_DCR_DDR8BNK		(1 << 3)
#define DDRP_DCR_TYPE			(0x7 << 0)
#define DDRP_DCR_TYPE_SHIFT		0
#define  DDRP_DCR_TYPE_MDDR		0
#define  DDRP_DCR_TYPE_DDR		1
#define  DDRP_DCR_TYPE_DDR2		2
#define  DDRP_DCR_TYPE_DDR3		3
#define  DDRP_DCR_TYPE_LPDDR2		4

/* MR0: Mode Register 0 */
#define DDRP_MR0_WR			(0x3 << 9)
#define DDRP_MR0_WR_SHIFT		9
#define DDRP_MR0_CL			(0x7 << 4)
#define DDRP_MR0_CL_SHIFT		4
#define DDRP_MR0_BT			(1 << 3)
#define DDRP_MR0_CL_GE12		(1 << 2)
#define DDRP_MR0_BL			(0x3 << 0)
#define DDRP_MR0_BL_SHIFT		0

/* MR1: Mode Register 1 */
#define DDRP_MR1_RTTH			(1 << 9)
#define DDRP_MR1_RTTM			(1 << 6)
#define DDRP_MR1_DICH			(1 << 5)
#define DDRP_MR1_RTTL			(1 << 2)
#define  DDRP_MR1_RTT_DIS		0
#define  DDRP_MR1_RTT_RZQ4		DDRP_MR1_RTTL
#define  DDRP_MR1_RTT_RZQ2		DDRP_MR1_RTTM
#define  DDRP_MR1_RTT_RZQ6		(DDRP_MR1_RTTM | DDRP_MR1_RTTL)
#define  DDRP_MR1_RTT_RZQ12		DDRP_MR1_RTTH
#define  DDRP_MR1_RTT_RZQ8		(DDRP_MR1_RTTH | DDRP_MR1_RTTL)
#define DDRP_MR1_DICL			(1 << 1)
#define  DDRP_MR1_DIC_RZQ6		0
#define  DDRP_MR1_DIC_RZQ7		DDRP_MR1_DICL

/* MR2: Mode Register 2 */
#define DDRP_MR2_CWL			(0x7 << 3)
#define DDRP_MR2_CWL_SHIFT		3

#define DDRP_MEM_PHY_BASE		0x20000000

#endif /* __JZ4780_DDR_PHY_H__ */
