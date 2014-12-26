/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_DWC2_H__
#define __FW_DWC2_H__

#include "io.h"

#define DWC2_BASE ((void *)0xb3500000)

#define DWC2_GEN_ACCESSORS(offset, name)		\
static inline uint32_t read_##name(void)		\
{							\
	return readl(DWC2_BASE + (offset));		\
}							\
							\
static inline void write_##name(uint32_t val)		\
{							\
	writel(val, DWC2_BASE + (offset));		\
}							\
							\
static inline uint32_t set_##name(uint32_t val)		\
{							\
	uint32_t tmp = read_##name();			\
	tmp |= (val);					\
	write_##name(tmp);				\
	return tmp;					\
}							\
							\
static inline uint32_t clear_##name(uint32_t val)	\
{							\
	uint32_t tmp = read_##name();			\
	tmp &= ~(val);					\
	write_##name(tmp);				\
	return tmp;					\
}

DWC2_GEN_ACCESSORS(0x0014, gint_sts)
DWC2_GEN_ACCESSORS(0x0018, gint_mask)
DWC2_GEN_ACCESSORS(0x001c, grxsts_read)
DWC2_GEN_ACCESSORS(0x0020, grxsts_pop)
DWC2_GEN_ACCESSORS(0x0818, otg_daint)
DWC2_GEN_ACCESSORS(0x0834, diep_empmsk)

#undef DWC2_GEN_ACCESSORS

#define DWC2_GEN_IDX_ACCESSORS(offset, stride, name)			\
static inline uint32_t read_##name(unsigned idx)			\
{									\
	return readl(DWC2_BASE + (offset) + ((idx) * (stride)));	\
}									\
									\
static inline void write_##name(unsigned idx, uint32_t val)		\
{									\
	writel(val, DWC2_BASE + (offset) + ((idx) * (stride)));		\
}									\
									\
static inline uint32_t set_##name(unsigned idx, uint32_t val)		\
{									\
	uint32_t tmp = read_##name(idx);				\
	tmp |= (val);							\
	write_##name(idx, tmp);						\
	return tmp;							\
}									\
									\
static inline uint32_t clear_##name(unsigned idx, uint32_t val)		\
{									\
	uint32_t tmp = read_##name(idx);				\
	tmp &= ~(val);							\
	write_##name(idx, tmp);						\
	return tmp;							\
}

DWC2_GEN_IDX_ACCESSORS(0x0900, 0x20, diep_ctl)
DWC2_GEN_IDX_ACCESSORS(0x0908, 0x20, diep_int)
DWC2_GEN_IDX_ACCESSORS(0x0910, 0x20, diep_size)
DWC2_GEN_IDX_ACCESSORS(0x0918, 0x20, dtxfsts)
DWC2_GEN_IDX_ACCESSORS(0x0b00, 0x20, doep_ctl)
DWC2_GEN_IDX_ACCESSORS(0x0b08, 0x20, doep_int)
DWC2_GEN_IDX_ACCESSORS(0x0b10, 0x20, doep_size)
DWC2_GEN_IDX_ACCESSORS(0x1000, 0x1000, ep_fifo)

#undef DWC2_GEN_IDX_ACCESSORS

/* gint_sts */
#define GINTSTS_START_FRAM		(1 << 3)
#define GINTSTS_RXFLVL			(1 << 4)
#define GINTSTS_USB_EARLYSUSPEND	(1 << 10)
#define GINTSTS_USB_RESET		(1 << 12)
#define GINTSTS_ENUM_DONE		(1 << 13)
#define GINTSTS_IEP_INTR		(1 << 18)
#define GINTSTS_OEP_INTR		(1 << 19)

/* grxsts[rp] */
#define GRXSTSP_EPNUM			(0xf << 0)
#define GRXSTSP_EPNUM_SHIFT		0
#define GRXSTSP_BYTE_CNT		(0x7ff << 4)
#define GRXSTSP_BYTE_CNT_SHIFT		4
#define GRXSTSP_PKSTS			(0xf << 17)
#define GRXSTSP_PKSTS_GOUT_NAK		(0x1 << 17)
#define GRXSTSP_PKSTS_GOUT_RECV		(0x2 << 17)
#define GRXSTSP_PKSTS_TX_COMP		(0x3 << 17)
#define GRXSTSP_PKSTS_SETUP_COMP	(0x4 << 17)
#define GRXSTSP_PKSTS_SETUP_RECV	(0x6 << 17)

/* d[io]ep_ctl */
#define DEP_CLEAR_NAK			(1 << 26)
#define DEP_ENA_BIT			(1 << 31)

/* d[io]ep_int */
#define DEP_XFER_COMP			(1 << 0)
#define DEP_SETUP_PHASE_DONE		(1 << 3)
#define DEP_TXFIFO_EMPTY		(1 << 7)
#define DEP_NAK_INT			(1 << 13)

/* doep_size */
#define DOEPSIZE0_PKTCNT_BIT		(1 << 19)

static inline unsigned dwc2_max_packet_size(unsigned ep)
{
	if (ep == 0)
		return 64;

	return 512;
}

#endif /* __FW_DWC2_H__ */
