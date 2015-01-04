/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JZ4780_REG_ACCESSORS_H__
#define __JZ4780_REG_ACCESSORS_H__

#include <setjmp.h>

#include "lib/ci20.h"

#define __GEN_ACCESSORS(addr, name, idx_decl, idx_arg)						\
static inline int read_##name(struct ci20_dev *dev, idx_decl uint32_t *val)			\
{												\
	return ci20_readl(dev, val, (addr));							\
}												\
												\
static inline uint32_t readj_##name(struct ci20_dev *dev, idx_decl jmp_buf env)			\
{												\
	uint32_t val;										\
	int err = read_##name(dev, idx_arg &val);						\
	if (err)										\
		longjmp(env, err);								\
	return val;										\
}												\
												\
static inline int write_##name(struct ci20_dev *dev, idx_decl uint32_t val)			\
{												\
	return ci20_writel(dev, val, (addr));							\
}												\
												\
static inline void writej_##name(struct ci20_dev *dev, idx_decl uint32_t val, jmp_buf env)	\
{												\
	int err = write_##name(dev, idx_arg val);						\
	if (err)										\
		longjmp(env, err);								\
}												\
												\
static inline int set_##name(struct ci20_dev *dev, idx_decl uint32_t bits)			\
{												\
	uint32_t val;										\
	int err = read_##name(dev, idx_arg &val);						\
	if (err)										\
		return err;									\
	val |= bits;										\
	return write_##name(dev, idx_arg val);							\
}												\
												\
static inline uint32_t setj_##name(struct ci20_dev *dev, idx_decl uint32_t bits, jmp_buf env)	\
{												\
	uint32_t val = readj_##name(dev, idx_arg env);						\
	val |= bits;										\
	writej_##name(dev, idx_arg val, env);							\
	return val;										\
}												\
												\
static inline int clear_##name(struct ci20_dev *dev, idx_decl uint32_t bits)			\
{												\
	uint32_t val;										\
	int err = read_##name(dev, idx_arg &val);						\
	if (err)										\
		return err;									\
	val &= ~bits;										\
	return write_##name(dev, idx_arg val);							\
}												\
												\
static inline uint32_t clearj_##name(struct ci20_dev *dev, idx_decl uint32_t bits, jmp_buf env)	\
{												\
	uint32_t val = readj_##name(dev, idx_arg env);						\
	val &= ~bits;										\
	writej_##name(dev, idx_arg val, env);							\
	return val;										\
}

#define GEN_ACCESSORS(addr, name) \
	__GEN_ACCESSORS(addr, name,,)

#define __IDX_DECL unsigned idx,
#define __IDX_ARG idx,

#define GEN_IDX_ACCESSORS(addr, stride, name) \
	__GEN_ACCESSORS(addr + (idx * (stride)), name, __IDX_DECL, __IDX_ARG)

#endif /* __JZ4780_REG_ACCESSORS_H__ */
