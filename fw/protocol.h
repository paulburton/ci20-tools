/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FW_PROTOCOL_H__
#define __FW_PROTOCOL_H__

#include <stdint.h>

#define FW_VERSION 0
#define FW_CPU_INFO "lci20-00"

enum {
	/* Compatible with Ingenic boot ROM */
	FW_REQ_GET_CPU_INFO = 0,

	/* Custom protocol */
	FW_REQ_MEM_READ,
	FW_REQ_MEM_WRITE,
	FW_REQ_MEM_SET,
	FW_REQ_CACHE_INIT,
	FW_REQ_CACHE_FLUSH,
	FW_REQ_MFC0,
	FW_REQ_MTC0,
	FW_REQ_JUMP,
	FW_REQ_BULK_LENGTH,
	FW_REQ_UART_INIT,
};

struct ci20_fw_mem_set {
	uint32_t length;
	uint32_t c;
};

enum {
	CACHE_D,
	CACHE_I,
};

struct ci20_fw_cache_flush {
	uint32_t base;
	uint32_t size;
};

#endif /* __FW_PROTOCOL_H__ */
