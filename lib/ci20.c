/*
 * libci20
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ci20.h"
#include "usb.h"

struct ci20_ctx {
	struct ci20_usb_ctx usb;
};

struct ci20_dev {
	struct ci20_usb_dev *usb_dev;
};

struct ci20_discover_ctx {
	struct ci20_ctx *c;
	void (*cb)(struct ci20_ctx *c, struct ci20_dev *dev, void *user);
	void *user;
};

struct ci20_ctx *ci20_init(void)
{
	struct ci20_ctx *c;
	int err;

	c = calloc(1, sizeof(*c));
	if (!c)
		return NULL;

	err = ci20_usb_init(&c->usb);
	if (err)
		goto out_err;

	return c;

out_err:
	ci20_fini(c);
	return NULL;
}

void ci20_fini(struct ci20_ctx *c)
{
	ci20_usb_fini(&c->usb);
	free(c);
}

static void ci20_discover_usb_cb(struct ci20_usb_ctx *usb, struct ci20_usb_dev *usb_dev, void *user)
{
	struct ci20_discover_ctx *ctx = user;
	struct ci20_dev *dev;

	dev = calloc(1, sizeof(*dev));
	if (!dev)
		return;

	dev->usb_dev = usb_dev;

	ctx->cb(ctx->c, dev, ctx->user);
}

int ci20_discover(struct ci20_ctx *c, void (*cb)(struct ci20_ctx *c, struct ci20_dev *dev, void *user), void *user)
{
	struct ci20_discover_ctx ctx = {
		.c = c,
		.cb = cb,
		.user = user,
	};
	return ci20_usb_discover(&c->usb, ci20_discover_usb_cb, &ctx);
}

int ci20_readmem(struct ci20_dev *dev, void *buf, size_t sz, uint32_t addr)
{
	return ci20_usb_readmem(dev->usb_dev, buf, sz, addr);
}

int ci20_readl(struct ci20_dev *dev, uint32_t *val, uint32_t addr)
{
	return ci20_readmem(dev, val, sizeof(*val), addr);
}

int ci20_writemem(struct ci20_dev *dev, const void *buf, size_t sz, uint32_t addr)
{
	return ci20_usb_writemem(dev->usb_dev, buf, sz, addr);
}

int ci20_writel(struct ci20_dev *dev, uint32_t val, uint32_t addr)
{
	return ci20_writemem(dev, &val, sizeof(val), addr);
}

int ci20_memset(struct ci20_dev *dev, uint32_t addr, uint8_t c, size_t n)
{
	return ci20_usb_memset(dev->usb_dev, addr, c, n);
}

int ci20_read_otp(struct ci20_dev *dev, struct ci20_otp *otp)
{
	int err;
	uint32_t status;
	unsigned attempts = 50;
	uint32_t efuse_base = 0xb34100d0;

	err = ci20_writel(dev, 0x00ff0000, efuse_base + 0x4);
	if (err)
		return err;

	err = ci20_writel(dev, 0x0, efuse_base + 0x8);
	if (err)
		return err;

	err = ci20_writel(dev, (0x18 << 21) | (15 << 16) | 0x1, efuse_base);
	if (err)
		return err;

	while (attempts--) {
		err = ci20_readl(dev, &status, efuse_base + 0x8);
		if (err)
			return err;

		if (status & 0x1)
			break;
	}

	return ci20_readmem(dev, otp, sizeof(*otp), efuse_base + 0xc);
}

int ci20_pin_config(struct ci20_dev *dev, unsigned port, unsigned pin, enum ci20_pin_func func)
{
	int err;

	/* PxINTC */
	err = ci20_writel(dev, 1 << pin, 0xb0010018 + (port * 0x100));
	if (err)
		return err;

	/* PxMSKC */
	err = ci20_writel(dev, 1 << pin, 0xb0010018 + (port * 0x100));
	if (err)
		return err;

	switch (func) {
	case PIN_DEVICE0:
	case PIN_DEVICE1:
	case PIN_GPIO_OUT_LOW:
	case PIN_GPIO_OUT_HIGH:
		/* PxPAT1C */
		err = ci20_writel(dev, 1 << pin, 0xb0010038 + (port * 0x100));
		break;

	case PIN_DEVICE2:
	case PIN_DEVICE3:
	case PIN_GPIO_IN:
		/* PxPAT1S */
		err = ci20_writel(dev, 1 << pin, 0xb0010034 + (port * 0x100));
		break;
	}
	if (err)
		return err;

	switch (func) {
	case PIN_DEVICE0:
	case PIN_DEVICE2:
	case PIN_GPIO_OUT_LOW:
		/* PxPAT0C */
		err = ci20_writel(dev, 1 << pin, 0xb0010048 + (port * 0x100));
		break;

	case PIN_DEVICE1:
	case PIN_DEVICE3:
	case PIN_GPIO_OUT_HIGH:
		/* PxPAT0S */
		err = ci20_writel(dev, 1 << pin, 0xb0010044 + (port * 0x100));
		break;

	case PIN_GPIO_IN:
		/* don't care */
		break;
	}

	return err;
}

int ci20_dcache_init(struct ci20_dev *dev)
{
	return ci20_usb_dcache_init(dev->usb_dev);
}

int ci20_icache_init(struct ci20_dev *dev)
{
	return ci20_usb_icache_init(dev->usb_dev);
}

int ci20_dcache_flush(struct ci20_dev *dev, uint32_t base, uint32_t size)
{
	return ci20_usb_dcache_flush(dev->usb_dev, base, size);
}

int ci20_icache_flush(struct ci20_dev *dev, uint32_t base, uint32_t size)
{
	return ci20_usb_icache_flush(dev->usb_dev, base, size);
}

int ci20_mfc0(struct ci20_dev *dev, unsigned reg, unsigned sel, uint32_t *val)
{
	return ci20_usb_mfc0(dev->usb_dev, reg, sel, val);
}

int ci20_mtc0(struct ci20_dev *dev, unsigned reg, unsigned sel, uint32_t val)
{
	return ci20_usb_mtc0(dev->usb_dev, reg, sel, val);
}

int ci20_set_k0_cca(struct ci20_dev *dev, unsigned cca)
{
	uint32_t val;
	int err;

	err = ci20_mfc0(dev, 16, 0, &val);
	if (err)
		return err;

	val &= ~0x7;
	val |= cca;

	return ci20_mtc0(dev, 16, 0, val);
}

int ci20_jump(struct ci20_dev *dev, uint32_t addr)
{
	return ci20_usb_jump(dev->usb_dev, addr);
}
