/*
 * libci20
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __usb_usb_h__
#define __usb_usb_h__

#include <stdint.h>
#include <libusb.h>

struct ci20_usb_ctx {
	libusb_context *ctx;
	struct ci20_usb_dev *devices;
};

struct ci20_usb_dev {
	struct ci20_usb_ctx *usb;
	struct ci20_usb_dev *prev, *next;

	libusb_device *ctx;
	libusb_device_handle *hnd;
	unsigned timeout;
};

extern int ci20_usb_init(struct ci20_usb_ctx *usb);
extern void ci20_usb_fini(struct ci20_usb_ctx *usb);

extern int ci20_usb_discover(struct ci20_usb_ctx *usb, void (*cb)(struct ci20_usb_ctx *usb, struct ci20_usb_dev *usb_dev, void *user), void *user);
extern void ci20_usb_close(struct ci20_usb_dev *dev);

extern int ci20_usb_readmem(struct ci20_usb_dev *dev, void *buf, size_t sz, uint32_t addr);
extern int ci20_usb_writemem(struct ci20_usb_dev *dev, const void *buf, size_t sz, uint32_t addr);

extern int ci20_usb_memset(struct ci20_usb_dev *dev, uint32_t addr, uint8_t c, size_t n);

extern int ci20_usb_dcache_init(struct ci20_usb_dev *dev);
extern int ci20_usb_icache_init(struct ci20_usb_dev *dev);

extern int ci20_usb_dcache_flush(struct ci20_usb_dev *dev, uint32_t base, uint32_t size);
extern int ci20_usb_icache_flush(struct ci20_usb_dev *dev, uint32_t base, uint32_t size);

extern int ci20_usb_mfc0(struct ci20_usb_dev *dev, unsigned reg, unsigned sel, uint32_t *val);
extern int ci20_usb_mtc0(struct ci20_usb_dev *dev, unsigned reg, unsigned sel, uint32_t val);

extern int ci20_usb_jump(struct ci20_usb_dev *dev, uint32_t addr);

extern int ci20_usb_uart_init(struct ci20_usb_dev *dev, unsigned uart, unsigned baud);

#endif /* __usb_usb_h__ */
