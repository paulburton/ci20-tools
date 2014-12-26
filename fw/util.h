/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_UTIL_H__
#define __FW_UTIL_H__

#define __ARG_PLACEHOLDER_1 0,
#define config_enabled(cfg) _config_enabled(cfg)
#define _config_enabled(value) __config_enabled(__ARG_PLACEHOLDER_##value)
#define __config_enabled(arg1_or_junk) ___config_enabled(arg1_or_junk 1, 0)
#define ___config_enabled(__ignored, val, ...) val

#define DIV_ROUND_UP(n, d) ({	\
	unsigned _n = (n);	\
	unsigned _d = (d);	\
	(_n + _d - 1) / _d;	\
})

#define min_t(type, a, b) ({	\
	type _a = (a);		\
	type _b = (b);		\
	_a < _b ? _a : _b;	\
})

#define clz __builtin_clz

#endif /* __FW_UTIL_H__ */
