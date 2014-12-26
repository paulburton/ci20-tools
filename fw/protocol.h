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

#define FW_VERSION 0
#define FW_CPU_INFO "lci20-00"

enum {
	/* Compatible with Ingenic boot ROM */
	FW_REQ_GET_CPU_INFO = 0,

	/* Custom protocol */
	FW_REQ_MEM_READ,
	FW_REQ_MEM_WRITE,
};

#endif /* __FW_PROTOCOL_H__ */
