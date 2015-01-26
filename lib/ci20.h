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

#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
	PIN_PULL_DISABLE	= (1 << 31),
};

enum ci20_pll {
	PLL_A = 0,
	PLL_M,
	PLL_E,
	PLL_V,
};

enum ci20_cpu_clk {
	CPU_CLK_NONE = 0,
	CPU_CLK_SCLK_A,
	CPU_CLK_PLL_M,
	CPU_CLK_PLL_E,
};

enum ci20_ddr_clk {
	DDR_CLK_NONE = 0,
	DDR_CLK_SCLK_A,
	DDR_CLK_PLL_M,
};

extern struct ci20_ctx *ci20_init(void);
extern void ci20_fini(struct ci20_ctx *c);

extern int ci20_discover(struct ci20_ctx *c, void (*cb)(struct ci20_ctx *c, struct ci20_dev *dev, void *user), void *user);

extern int ci20_readmem(struct ci20_dev *dev, void *buf, size_t sz, uint32_t addr);
extern int ci20_readb(struct ci20_dev *dev, uint8_t *val, uint32_t addr);
extern int ci20_readl(struct ci20_dev *dev, uint32_t *val, uint32_t addr);

extern int ci20_writemem(struct ci20_dev *dev, const void *buf, size_t sz, uint32_t addr);
extern int ci20_writeb(struct ci20_dev *dev, uint8_t val, uint32_t addr);
extern int ci20_writel(struct ci20_dev *dev, uint32_t val, uint32_t addr);

extern int ci20_memset(struct ci20_dev *dev, uint32_t addr, uint8_t c, size_t n);

extern int ci20_read_otp(struct ci20_dev *dev, struct ci20_otp *otp);

extern int ci20_pin_config(struct ci20_dev *dev, unsigned port, unsigned pin, enum ci20_pin_func func);
extern int ci20_pin_level(struct ci20_dev *dev, unsigned port, unsigned pin, int *level);

extern int ci20_pll_init(struct ci20_dev *dev, enum ci20_pll pll, unsigned m, unsigned n, unsigned od);
extern int ci20_mux_cpu_clk(struct ci20_dev *dev, enum ci20_cpu_clk clk);
extern int ci20_mux_ddr_clk(struct ci20_dev *dev, enum ci20_ddr_clk clk, unsigned div);

extern int ci20_ddr_init(struct ci20_dev *dev, unsigned hz);

extern int ci20_dcache_init(struct ci20_dev *dev);
extern int ci20_icache_init(struct ci20_dev *dev);

extern int ci20_dcache_flush(struct ci20_dev *dev, uint32_t base, uint32_t size);
extern int ci20_icache_flush(struct ci20_dev *dev, uint32_t base, uint32_t size);

extern int ci20_mfc0(struct ci20_dev *dev, unsigned reg, unsigned sel, uint32_t *val);
extern int ci20_mtc0(struct ci20_dev *dev, unsigned reg, unsigned sel, uint32_t val);

extern int ci20_set_k0_cca(struct ci20_dev *dev, unsigned cca);

extern int ci20_load_elf(struct ci20_dev *dev, const void *base, uint32_t *entry);
extern int ci20_load_elf_fd(struct ci20_dev *dev, int filedes, uint32_t *entry);
extern int ci20_load_elf_path(struct ci20_dev *dev, const char *path, uint32_t *entry);

extern int ci20_jump(struct ci20_dev *dev, uint32_t addr);

extern int ci20_uart_init(struct ci20_dev *dev, unsigned uart, unsigned baud);

extern int ci20_nand_init(struct ci20_dev *dev);
extern int ci20_nand_id(struct ci20_dev *dev, unsigned char id[static 6]);

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
