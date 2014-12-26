/*
 * libci20
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __usb_ci20_h__
#define __usb_ci20_h__

#include <stdint.h>

struct ci20_ctx;
struct ci20_dev;

struct ci20_otp {
	uint32_t serial;
	uint32_t manufacture_date;
	char manufacturer[2];
	unsigned char mac[6];
} __attribute__((packed));

enum ci20_pin_func {
	PIN_DEVICE0,
	PIN_DEVICE1,
	PIN_DEVICE2,
	PIN_DEVICE3,
	PIN_GPIO_IN,
	PIN_GPIO_OUT_LOW,
	PIN_GPIO_OUT_HIGH,
};

extern struct ci20_ctx *ci20_init(void);
extern void ci20_fini(struct ci20_ctx *c);

extern int ci20_discover(struct ci20_ctx *c, void (*cb)(struct ci20_ctx *c, struct ci20_dev *dev, void *user), void *user);

extern int ci20_readmem(struct ci20_dev *dev, void *buf, size_t sz, uint32_t addr);
extern int ci20_readl(struct ci20_dev *dev, uint32_t *val, uint32_t addr);

extern int ci20_writemem(struct ci20_dev *dev, const void *buf, size_t sz, uint32_t addr);
extern int ci20_writel(struct ci20_dev *dev, uint32_t val, uint32_t addr);

extern int ci20_read_otp(struct ci20_dev *dev, struct ci20_otp *otp);

extern int ci20_pin_config(struct ci20_dev *dev, unsigned port, unsigned pin, enum ci20_pin_func func);

/**
 * ci20_manufacturer_long - retrieve long manufacturer name from short
 * @mfr: The short manufacturer name, as read from the OTP.
 * @def: The value to return if mfr is unknown.
 *
 * Return a long form of the manufacturer name mfr, or default if the
 * manufacturer is not recognised.
 */
static inline const char *ci20_manufacturer_long(const char mfr[2], const char *def)
{
	if (!strncmp(mfr, "NP", 2))
		return "Nopa";

	return def;
}

#endif /* __usb_ci20_h__ */
