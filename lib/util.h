/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LIB_UTIL_H__
#define __LIB_UTIL_H__

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define DIV_ROUND_UP(n, d) ({	\
	typeof(n) _n = (n);	\
	typeof(d) _d = (d);	\
	(_n + _d - 1) / _d;	\
})

#define max_t(type, a, b) ({	\
	type _a = (a);		\
	type _b = (b);		\
	_a > _b ? _a : _b;	\
})

#define max(a, b)		\
	max_t(typeof(a), a, b)

#define min_t(type, a, b) ({	\
	type _a = (a);		\
	type _b = (b);		\
	_a < _b ? _a : _b;	\
})

#define min(a, b)		\
	min_t(typeof(a), a, b)

#define bound_t(type, min, val, max) ({			\
	type _min = (min);				\
	type _val = (val);				\
	type _max = (max);				\
	min_t(type, max_t(type, _val, _min), _max);	\
})

#define bound(min, val, max)	\
	bound_t(typeof(val), min, val, max)

#define clz __builtin_clz
#define ffs __builtin_ffs

#endif /* __LIB_UTIL_H__ */
