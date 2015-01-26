/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LIBCI20_NAND_H__
#define __LIBCI20_NAND_H__

#include "reg-accessors.h"

#define NAND_GEN_IOACCESSORS(addr, name)							\
static inline int read_nand_##name(struct ci20_dev *dev, uint8_t *val)				\
{												\
	return ci20_readb(dev, val, (addr));							\
}												\
												\
static inline uint8_t readj_nand_##name(struct ci20_dev *dev, jmp_buf env)			\
{												\
	uint8_t val;										\
	int err = read_nand_##name(dev, &val);							\
	if (err)										\
		longjmp(env, err);								\
	return val;										\
}												\
												\
static inline int write_nand_##name(struct ci20_dev *dev, uint8_t val)				\
{												\
	return ci20_writeb(dev, val, (addr));							\
}												\
												\
static inline void writej_nand_##name(struct ci20_dev *dev, uint8_t val, jmp_buf env)		\
{												\
	int err = write_nand_##name(dev, val);							\
	if (err)										\
		longjmp(env, err);								\
}

NAND_GEN_IOACCESSORS(0xbb000000, data)
NAND_GEN_IOACCESSORS(0xbb400000, cmd)
NAND_GEN_IOACCESSORS(0xbb800000, addr)

#undef NAND_GEN_IOACCESSORS

#define NEMC_BASE 0xb3410000

#define NEMC_GEN_ACCESSORS(offset, name) \
	GEN_ACCESSORS(NEMC_BASE + (offset), nemc_##name)

NEMC_GEN_ACCESSORS(0x014, smcr1)
NEMC_GEN_ACCESSORS(0x050, nfcsr)
NEMC_GEN_ACCESSORS(0x100, pncr)

#undef NEMC_GEN_ACCESSORS

/* NFCSR */
#define NEMC_NFCSR_NFCE1	(1 << 1)
#define NEMC_NFCSR_NFE1		(1 << 0)

#endif /* __LIBCI20_NAND_H__ */
