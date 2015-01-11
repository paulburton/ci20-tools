/*
 * libci20
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "fw/protocol.h"

#include "usb.h"
#include "util.h"

/* Ingenic boot ROM vendor requests */
#define INGENIC_VR_GET_CPU_INFO		0x00
#define INGENIC_VR_SET_DATA_ADDRESS	0x01
#define INGENIC_VR_PROGRAM_START1	0x04
#define INGENIC_VR_PROGRAM_START2	0x05

int ci20_usb_init(struct ci20_usb_ctx *usb)
{
	int err;

	err = libusb_init(&usb->ctx);
	if (err)
		return err;

	libusb_set_debug(usb->ctx, LIBUSB_LOG_LEVEL_WARNING);

	return 0;
}

void ci20_usb_fini(struct ci20_usb_ctx *usb)
{
	while (usb->devices)
		ci20_usb_close(usb->devices);

	libusb_exit(usb->ctx);
}

static int ci20_get_cpu_info(struct ci20_usb_dev *dev, char cpu_info[static 8])
{
	int transferred;

	transferred = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		INGENIC_VR_GET_CPU_INFO, 0, 0, (unsigned char *)cpu_info, 8, dev->timeout);
	if (transferred != 8)
		return -EIO;

	return 0;
}

static int ci20_usb_load_fw(struct ci20_usb_dev *dev)
{
	extern unsigned char _binary_out_fw_fw_bin_start[];
	extern unsigned char _binary_out_fw_fw_bin_end[];
	size_t sz = _binary_out_fw_fw_bin_end - _binary_out_fw_fw_bin_start;
	uint32_t base = 0xf4000800;
	int err, transferred;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		INGENIC_VR_SET_DATA_ADDRESS, base >> 16, base, NULL, 0, dev->timeout);
	if (err)
		return err;

	err = libusb_bulk_transfer(dev->hnd, LIBUSB_ENDPOINT_OUT | 0x1,
		_binary_out_fw_fw_bin_start, sz, &transferred, dev->timeout);
	if (err)
		return err;
	if (transferred != sz)
		return -EIO;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		INGENIC_VR_PROGRAM_START1, base >> 16, base, NULL, 0, dev->timeout);
	if (err)
		return err;

	return 0;
}

int ci20_usb_discover(struct ci20_usb_ctx *usb, void (*cb)(struct ci20_usb_ctx *usb, struct ci20_usb_dev *usb_dev, void *user), void *user)
{
	size_t i;
	ssize_t count;
	libusb_device **ds;
	libusb_device_handle *hnd;
	struct libusb_device_descriptor desc;
	struct ci20_usb_dev *usb_dev;
	char cpu_info[8];
	int err;

	count = libusb_get_device_list(usb->ctx, &ds);
	if (count < 0)
		return count;

	for (i = 0; i < count; i++) {
		usb_dev = NULL;

		err = libusb_get_device_descriptor(ds[i], &desc);
		if (err)
			continue;

		if (desc.idVendor != 0xa108)
			continue;
		if (desc.idProduct != 0x4780)
			continue;

		err = libusb_open(ds[i], &hnd);
		if (err)
			continue;

		err = libusb_claim_interface(hnd, 0);
		if (err) {
dev_err:
			libusb_close(hnd);
			free(usb_dev);
			continue;
		}

		usb_dev = calloc(1, sizeof(*usb_dev));
		if (!usb_dev)
			goto dev_err;

		usb_dev->usb = usb;
		usb_dev->ctx = libusb_ref_device(ds[i]);
		usb_dev->hnd = hnd;
		usb_dev->timeout = 500;

		err = ci20_get_cpu_info(usb_dev, cpu_info);
		if (err)
			goto dev_err;

		if (!strncmp(cpu_info, "JZ4780V1", sizeof(cpu_info))) {
			err = ci20_usb_load_fw(usb_dev);
			if (err)
				goto dev_err;

			err = ci20_get_cpu_info(usb_dev, cpu_info);
			if (err)
				goto dev_err;
		}

		if (strncmp(cpu_info, FW_CPU_INFO, sizeof(cpu_info)))
			goto dev_err;

		if (!usb->devices) {
			usb->devices = usb_dev->prev = usb_dev->next = usb_dev;
		} else {
			usb_dev->prev = usb->devices;
			usb_dev->next = usb->devices->next;
			usb_dev->prev->next = usb_dev;
			usb_dev->next->prev = usb_dev;
		}

		cb(usb, usb_dev, user);
	}

	libusb_free_device_list(ds, 1);
	return 0;
}

void ci20_usb_close(struct ci20_usb_dev *dev)
{
	if (dev->next == dev) {
		dev->usb->devices = NULL;
	} else {
		dev->prev->next = dev->next;
		dev->next->prev = dev->prev;

		if (dev->usb->devices == dev)
			dev->usb->devices = dev->next;
	}

	libusb_close(dev->hnd);
	free(dev);
}

int ci20_usb_readmem(struct ci20_usb_dev *dev, void *buf, size_t sz, uint32_t addr)
{
	int err;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_MEM_READ, addr >> 16, addr, (unsigned char *)buf, sz, dev->timeout);
	if (err < 0)
		return err;
	if (err != sz)
		return -EIO;
	return 0;
}

int ci20_usb_writemem(struct ci20_usb_dev *dev, const void *buf, size_t sz, uint32_t addr)
{
	const unsigned ctl_data_max = 64;
	const unsigned bulk_data_max = 1 << 16;
	uint16_t ctl_data_len = 0;
	unsigned char *ctl_data = NULL;
	int err, transferred;

	if (sz <= ctl_data_max) {
		ctl_data = (unsigned char *)buf;
		ctl_data_len = sz;
	}

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_MEM_WRITE, addr >> 16, addr, ctl_data, ctl_data_len, dev->timeout);
	if (err < 0)
		return err;
	if (err != ctl_data_len)
		return -EIO;

	if (sz <= ctl_data_max)
		return 0;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_BULK_LENGTH, sz >> 16, sz, NULL, 0, dev->timeout);
	if (err)
		return err;

	while (sz) {
		err = libusb_bulk_transfer(dev->hnd, LIBUSB_ENDPOINT_OUT | 0x1,
					   (unsigned char *)buf, min(sz, bulk_data_max),
					   &transferred, dev->timeout);

		if (err && (err != LIBUSB_ERROR_TIMEOUT))
			return err;

		buf += transferred;
		sz -= transferred;
	}

	return 0;
}

int ci20_usb_memset(struct ci20_usb_dev *dev, uint32_t addr, uint8_t c, size_t n)
{
	struct ci20_fw_mem_set args = {
		.c = c,
		.length = n,
	};
	int err;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_MEM_SET, addr >> 16, addr, (unsigned char *)&args, sizeof(args),
		dev->timeout);
	if (err < 0)
		return err;
	if (err != sizeof(args))
		return err;

	return 0;
}

int ci20_usb_dcache_init(struct ci20_usb_dev *dev)
{
	return libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_CACHE_INIT, CACHE_D, 0, NULL, 0, dev->timeout);
}

int ci20_usb_icache_init(struct ci20_usb_dev *dev)
{
	return libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_CACHE_INIT, CACHE_I, 0, NULL, 0, dev->timeout);
}

int ci20_usb_dcache_flush(struct ci20_usb_dev *dev, uint32_t base, uint32_t size)
{
	struct ci20_fw_cache_flush args = {
		.base = base,
		.size = size,
	};
	int err;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_CACHE_FLUSH, CACHE_D, 0, (unsigned char *)&args, sizeof(args),
		dev->timeout);
	if (err < 0)
		return err;
	if (err != sizeof(args))
		return err;

	return 0;
}

int ci20_usb_icache_flush(struct ci20_usb_dev *dev, uint32_t base, uint32_t size)
{
	struct ci20_fw_cache_flush args = {
		.base = base,
		.size = size,
	};
	int err;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_CACHE_FLUSH, CACHE_I, 0, (unsigned char *)&args, sizeof(args),
		dev->timeout);
	if (err < 0)
		return err;
	if (err != sizeof(args))
		return err;

	return 0;
}

int ci20_usb_mfc0(struct ci20_usb_dev *dev, unsigned reg, unsigned sel, uint32_t *val)
{
	int err;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_MFC0, reg, sel, (unsigned char *)val, sizeof(*val),
		dev->timeout);
	if (err < 0)
		return err;
	if (err != sizeof(*val))
		return err;

	return 0;
}

int ci20_usb_mtc0(struct ci20_usb_dev *dev, unsigned reg, unsigned sel, uint32_t val)
{
	int err;

	err = libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_MTC0, reg, sel, (unsigned char *)&val, sizeof(val),
		dev->timeout);
	if (err < 0)
		return err;
	if (err != sizeof(val))
		return err;

	return 0;
}

int ci20_usb_jump(struct ci20_usb_dev *dev, uint32_t addr)
{
	return libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_JUMP, addr >> 16, addr, NULL, 0, dev->timeout);
}

int ci20_usb_uart_init(struct ci20_usb_dev *dev, unsigned uart, unsigned baud)
{
	if (baud % 100)
		return -EINVAL;

	return libusb_control_transfer(dev->hnd,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		FW_REQ_UART_INIT, uart, baud / 100, NULL, 0, dev->timeout);
}
