/*
 * libci20 Firmware
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_CACHE_H__
#define __FW_CACHE_H__

#include <stddef.h>
#include <stdint.h>

extern void init_icache(void);
extern void init_dcache(void);
extern void flush_icache(uintptr_t base, size_t size);
extern void flush_dcache(uintptr_t base, size_t size);

#endif /* __FW_CACHE_H__ */
