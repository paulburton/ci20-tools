/*
 * libci20 Firmware
 * Copyright (C) 2014-2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stddef.h>

#include "cache.h"
#include "cop0.h"
#include "debug.h"
#include "dwc2.h"
#include "protocol.h"
#include "usb.h"
#include "util.h"

enum {
	REQ_GET_DESCRIPTOR = 0x6,
};

enum {
	DT_DEVICE = 0x1,
	DT_CONFIG,
	DT_STRING,
};

static union {
	struct {
		uint8_t bmRequestType;
		uint8_t bRequest;
		uint16_t wValue;
		uint16_t wIndex;
		uint16_t wLength;
	} __attribute__((packed));

	uint32_t raw[2];
} setup_packet;

static struct {
	const void *data;
	uint32_t size;
} in_data;

static struct {
	const void *data;
	uint32_t size;
} out_data;

static union {
	struct {
		struct ci20_fw_mem_set args;
		void *base;
	} mem_set;

	struct {
		struct ci20_fw_cache_flush args;
		uint8_t cache;
	} cache_flush;

	struct {
		uint32_t value;
	} mfc0;

	struct {
		unsigned reg, sel;
		uint32_t value;
	} mtc0;

	struct {
		uintptr_t addr;
	} jump;
} cmd_data;

static int cmd = -1, prev_cmd = -1;

static const char cpu_info[8] __attribute__((aligned(4))) = FW_CPU_INFO;

static const unsigned short str_manufacturer[] = {
	(DT_STRING << 8) | (25 * 2),
	'I', 'm', 'a', 'g', 'i', 'n', 'a', 't', 'i', 'o', 'n', ' ',
	'T', 'e', 'c', 'h', 'n', 'o', 'l', 'o', 'g', 'i', 'e', 's',
};

static const unsigned short str_product[] = {
	(DT_STRING << 8) | (18 * 2),
	'M', 'I', 'P', 'S', ' ', 'C', 'r', 'e', 'a', 't', 'o', 'r', ' ', 'C', 'I', '2', '0',
};

static void handle_descriptor_request(void)
{
	uint8_t type, idx;

	type = setup_packet.wValue >> 8;
	idx = setup_packet.wValue & 0xff;

	switch (type) {
	case DT_STRING:
		switch (idx) {
		case 1:
			in_data.data = str_manufacturer;
			in_data.size = sizeof(str_manufacturer);
			break;

		case 2:
			in_data.data = str_product;
			in_data.size = sizeof(str_product);
			break;

		default:
			debug("WARN: unhandled string descriptor 0x");
			debug_hex(idx, 0);
			debug("\r\n");
		}
		break;

	default:
		debug("WARN: unhandled descriptor type 0x");
		debug_hex(type, 0);
		debug("\r\n");
	}
}

static void handle_standard_request(void)
{
	switch (setup_packet.bRequest) {
	case REQ_GET_DESCRIPTOR:
		handle_descriptor_request();
		break;

	default:
		debug("WARN: unhandled standard request 0x");
		debug_hex(setup_packet.bRequest, 2);
		debug("\r\n");
	}
}

static void handle_vendor_request(void)
{
	uint32_t u32val;
	unsigned baud;

	cmd = setup_packet.bRequest;

	u32val = (uint32_t)setup_packet.wValue << 16;
	u32val |= setup_packet.wIndex;

	switch (cmd) {
	case FW_REQ_GET_CPU_INFO:
		in_data.data = cpu_info;
		in_data.size = sizeof(cpu_info);

		debug("GET_CPU_INFO\r\n");
		break;

	case FW_REQ_MEM_READ:
		in_data.data = (const void *)u32val;
		in_data.size = setup_packet.wLength;

		debug("MEM_READ addr=0x");
		debug_hex((uint32_t)in_data.data, 8);
		debug(" length=0x");
		debug_hex(in_data.size, 0);
		debug("\r\n");
		break;

	case FW_REQ_MEM_WRITE:
		out_data.data = (const void *)u32val;
		out_data.size = setup_packet.wLength;

		debug("MEM_WRITE addr=0x");
		debug_hex((uint32_t)out_data.data, 8);
		debug(" length=0x");
		debug_hex(out_data.size, 0);
		debug("\r\n");
		break;

	case FW_REQ_MEM_SET:
		out_data.data = (void *)&cmd_data.mem_set;
		out_data.size = sizeof(cmd_data.mem_set.args);
		cmd_data.mem_set.base = (void *)u32val;

		debug("MEM_SET addr=0x");
		debug_hex((uint32_t)cmd_data.mem_set.base, 8);
		debug("\r\n");
		break;

	case FW_REQ_CACHE_INIT:
		debug("CACHE_INIT");

		switch (setup_packet.wValue) {
		case CACHE_D:
			debug(" DCACHE\r\n");
			init_dcache();
			break;

		case CACHE_I:
			debug(" ICACHE\r\n");
			init_icache();
			break;

		default:
			debug(" unknown=0x");
			debug_hex(setup_packet.wValue, 0);
			debug("\r\n");
			break;
		}
		break;

	case FW_REQ_CACHE_FLUSH:
		debug("CACHE_FLUSH");

		out_data.data = (void *)&cmd_data.cache_flush.args;
		out_data.size = sizeof(cmd_data.cache_flush.args);
		cmd_data.cache_flush.cache = setup_packet.wValue;

		switch (setup_packet.wValue) {
		case CACHE_D:
			debug(" DCACHE\r\n");
			break;

		case CACHE_I:
			debug(" ICACHE\r\n");
			break;

		default:
			debug(" unknown=0x");
			debug_hex(setup_packet.wValue, 0);
			debug("\r\n");
			break;
		}
		break;

	case FW_REQ_MFC0:
		debug("MFC0 reg=0x");
		debug_hex(setup_packet.wValue, 0);
		debug(" sel=0x");
		debug_hex(setup_packet.wIndex, 0);
		debug("\r\n");

		cmd_data.mfc0.value = dynamic_mfc0(setup_packet.wValue,
						   setup_packet.wIndex);

		in_data.data = &cmd_data.mfc0.value;
		in_data.size = sizeof(cmd_data.mfc0.value);
		break;

	case FW_REQ_MTC0:
		debug("MTC0 reg=0x");
		debug_hex(setup_packet.wValue, 0);
		debug(" sel=0x");
		debug_hex(setup_packet.wIndex, 0);
		debug("\r\n");

		cmd_data.mtc0.reg = setup_packet.wValue;
		cmd_data.mtc0.sel = setup_packet.wIndex;

		out_data.data = &cmd_data.mtc0.value;
		out_data.size = sizeof(cmd_data.mtc0.value);
		break;

	case FW_REQ_JUMP:
		debug("JUMP addr=0x");
		debug_hex(u32val, 8);
		debug("\r\n");

		cmd_data.jump.addr = u32val;
		break;

	case FW_REQ_BULK_LENGTH:
		debug("BULK_LENGTH len=0x");
		debug_hex(u32val, 8);
		debug("\r\n");

		switch (prev_cmd) {
		case FW_REQ_MEM_READ:
			in_data.size = u32val;
			break;

		case FW_REQ_MEM_WRITE:
			out_data.size = u32val;
			break;

		default:
			debug("   INVALID!\r\n");
			break;
		}
		break;

	case FW_REQ_UART_INIT:
		baud = setup_packet.wIndex * 100;

		debug("UART_INIT uart=0x");
		debug_hex(setup_packet.wValue, 0);
		debug(" baud=0x");
		debug_hex(baud, 0);
		debug("\r\n");

		uart_init(setup_packet.wValue, baud);
		break;
	}
}

static void handle_setup_packet(unsigned ep)
{
	void (*fn_ptr)(unsigned a0, unsigned a1, unsigned a2, unsigned a3);
	unsigned type = (setup_packet.bmRequestType >> 5) & 0x3;
	unsigned pkt_cnt;

	prev_cmd = cmd;
	cmd = -1;

	switch (type) {
	case 0:
		handle_standard_request();
		break;

	case 2:
		handle_vendor_request();
		break;
	}

	if (in_data.size) {
		pkt_cnt = DIV_ROUND_UP(in_data.size, dwc2_max_packet_size(ep));
		set_diep_size(ep, (pkt_cnt << 19) | in_data.size);
		set_diep_empmsk(1 << ep);
	} else {
		set_diep_size(ep, DOEPSIZE0_PKTCNT_BIT);
	}

	set_diep_ctl(ep, DEP_ENA_BIT | DEP_CLEAR_NAK);

	if (out_data.size)
		set_doep_ctl(ep, DEP_ENA_BIT | DEP_CLEAR_NAK);

	if (cmd == FW_REQ_JUMP) {
		fn_ptr = (void *)cmd_data.jump.addr;
		fn_ptr(0, 0, 0, 0);
	}
}

static void handle_earlysuspend_interrupt(void)
{
	set_gint_sts(GINTSTS_USB_EARLYSUSPEND);
}

static void handle_start_fram_interrupt(void)
{
	set_gint_sts(GINTSTS_START_FRAM);
}

static void handle_usb_reset_interrupt(void)
{
	debug("usb_reset\r\n");
	set_gint_sts(GINTSTS_USB_RESET);
}

static void handle_enum_done_interrupt(void)
{
	debug("enum_done\r\n");
	set_gint_sts(GINTSTS_ENUM_DONE);
}

static void *memset(void *s, int c, size_t n)
{
	uint8_t *pchar = s;
	size_t i;

	for (i = 0; i < n; i++)
		pchar[i] = c;

	return s;
}

static void read_out_data(unsigned ep, unsigned sz)
{
	unsigned dwords = DIV_ROUND_UP(sz, 4);
	uint32_t val;

	while (dwords--) {
		val = read_ep_fifo(ep);

		if (out_data.size >= 4) {
			debug("<< 0x");
			debug_hex(val, 8);
			debug("\r\n");

			*((volatile uint32_t *)out_data.data) = val;
			out_data.data += 4;
			out_data.size -= 4;
		} else while (out_data.size > 0) {
			debug("<< 0x");
			debug_hex(val & 0xff, 2);
			debug("\r\n");

			*((volatile uint8_t *)out_data.data) = val;
			val >>= 8;
			out_data.data++;
			out_data.size--;
		}
	}

	if (out_data.size)
		return;

	switch (cmd) {
	case FW_REQ_MEM_SET:
		memset(cmd_data.mem_set.base, cmd_data.mem_set.args.c,
		       cmd_data.mem_set.args.length);
		break;

	case FW_REQ_CACHE_FLUSH:
		switch (cmd_data.cache_flush.cache) {
		case CACHE_D:
			flush_dcache(cmd_data.cache_flush.args.base,
				     cmd_data.cache_flush.args.size);
			break;

		case CACHE_I:
			flush_icache(cmd_data.cache_flush.args.base,
				     cmd_data.cache_flush.args.size);
			break;
		}
		break;

	case FW_REQ_MTC0:
		dynamic_mtc0(cmd_data.mtc0.reg, cmd_data.mtc0.sel,
			     cmd_data.mtc0.value);
		break;
	}
}

static void handle_rxflvl_interrupt(void)
{
	uint32_t rxsts = read_grxsts_pop();
	unsigned sz, ep, pktsts;

	pktsts = (rxsts & GRXSTS_PKTSTS) >> GRXSTS_PKTSTS_SHIFT;
	ep = (rxsts & GRXSTS_EPNUM) >> GRXSTS_EPNUM_SHIFT;

	switch (pktsts) {
	case GRXSTS_PKTSTS_OUT_RECV:
		sz = (rxsts & GRXSTS_BCNT) >> GRXSTS_BCNT_SHIFT;
		read_out_data(ep, sz);
		break;

	case GRXSTS_PKTSTS_SETUP_RECV:
		setup_packet.raw[0] = read_ep_fifo(ep);
		setup_packet.raw[1] = read_ep_fifo(ep);
		break;
	}

	set_gint_sts(GINTSTS_RXFLVL);
}

static void handle_iep_interrupt(void)
{
	uint16_t intrs = read_otg_daint() & 0xffff;
	uint32_t ep_intr, txstatus, short_data;
	unsigned ep, xfersize, dwords, xfered;

	while (intrs) {
		ep = 31 - clz(intrs);
		ep_intr = read_diep_int(ep);
		intrs &= ~(1 << ep);

		if (!(ep_intr & DEP_TXFIFO_EMPTY))
			continue;

		if (!(read_diep_empmsk() & (1 << ep)))
			continue;

		xfered = 0;

		while (in_data.size) {
			xfersize = min_t(unsigned, in_data.size, dwc2_max_packet_size(ep));
			dwords = DIV_ROUND_UP(xfersize, 4);
			txstatus = read_dtxfsts(ep);

			if (txstatus < dwords)
				break;

			while (xfersize >= 4) {
				volatile uint32_t *ptr = (uint32_t *)in_data.data;
				uint32_t val = *ptr;

				debug(">> 0x");
				debug_hex(val, 8);
				debug("\r\n");
				write_ep_fifo(ep, val);

				in_data.data += 4;
				in_data.size -= 4;
				xfered += 4;
				xfersize -= 4;
			}

			if (!xfersize)
				break;

			short_data = 0;
			while (xfersize > 0) {
				volatile uint8_t *ptr = (uint8_t *)in_data.data;

				short_data <<= 8;
				short_data |= *ptr;

				in_data.data++;
				in_data.size--;
				xfered++;
				xfersize--;
			}

			debug(">> 0x");
			debug_hex(short_data, (xfered % 4) * 2);
			debug("\r\n");
			write_ep_fifo(ep, short_data);
		}

		set_diep_int(ep, DEP_TXFIFO_EMPTY);

		if (xfered) {
			while (!(read_diep_int(ep) & DEP_XFER_COMP));
			set_diep_int(ep, DEP_XFER_COMP);
			clear_diep_empmsk(1 << ep);
			set_doep_ctl(ep, DEP_ENA_BIT | DEP_CLEAR_NAK);
			if (ep_intr & DEP_NAK_INT)
				set_diep_int(ep, DEP_NAK_INT);
		}
	}
}

static void handle_oep_interrupt(void)
{
	uint16_t intrs = read_otg_daint() >> 16;
	uint32_t ep_intr;
	unsigned ep;

	while (intrs) {
		ep = 31 - clz(intrs);
		ep_intr = read_doep_int(ep);
		intrs &= ~(1 << ep);

		if (ep_intr & DEP_XFER_COMP) {
			set_doep_int(ep, DEP_XFER_COMP);

			if (out_data.size)
				set_doep_ctl(ep, DEP_ENA_BIT | DEP_CLEAR_NAK);
		}

		if (ep_intr & DEP_SETUP_PHASE_DONE) {
			set_doep_int(ep, DEP_SETUP_PHASE_DONE);
			handle_setup_packet(ep);
		}
	}
}

void usb_poll(void)
{
	uint32_t sts = read_gint_sts();

	if (sts & GINTSTS_USB_EARLYSUSPEND)
		handle_earlysuspend_interrupt();

	if (sts & GINTSTS_START_FRAM)
		handle_start_fram_interrupt();

	if (sts & GINTSTS_USB_RESET)
		handle_usb_reset_interrupt();

	if (sts & GINTSTS_ENUM_DONE)
		handle_enum_done_interrupt();

	if (sts & GINTSTS_IEP_INTR)
		handle_iep_interrupt();

	if (sts & GINTSTS_OEP_INTR)
		handle_oep_interrupt();

	if (sts & GINTSTS_RXFLVL)
		handle_rxflvl_interrupt();
}
