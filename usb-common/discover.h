/*
 * ci20-tools
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __COMMON_DISCOVER_H__
#define __COMMON_DISCOVER_H__

#include <stdbool.h>

extern struct ci20_ctx *ctx;
extern struct ci20_dev *dev;
extern struct ci20_otp otp;

extern int common_init(int serial, bool wait);

#endif /* __COMMON_DISCOVER_H__ */
