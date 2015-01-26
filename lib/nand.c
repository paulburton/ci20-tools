/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "ci20.h"
#include "jz4780/cgu.h"
#include "jz4780/nand.h"
#include "util.h"

static int wait_ready(struct ci20_dev *dev)
{
	int err, level;
	unsigned timeout = 1000;

	do {
		err = ci20_pin_level(dev, 0, 20, &level);
		if (err)
			return err;
	} while (level && timeout--);

	do {
		err = ci20_pin_level(dev, 0, 20, &level);
		if (err)
			return err;
	} while (!level);

	return 0;
}

static int read_buf(struct ci20_dev *dev, void *buf, size_t sz)
{
	uint8_t *u8buf = buf;
	int err;

	while (sz--) {
		err = read_nand_data(dev, u8buf++);
		if (err)
			return err;
	}

	return 0;
}

int ci20_nand_init(struct ci20_dev *dev)
{
	const int func0_pins[] = {
		0, 1, 2, 3, 4, 5, 6, 7,
		18, 19, 21, 32, 33,
	};
	unsigned i;
	uint8_t status;
	jmp_buf env;
	int err;

	err = setjmp(env);
	if (err)
		return err;

	for (i = 0; i < ARRAY_SIZE(func0_pins); i++) {
		err = ci20_pin_config(dev, func0_pins[i] / 32,
				      func0_pins[i] % 32, PIN_DEVICE0 | PIN_PULL_DISABLE);
		if (err)
			return err;
	}

	/* FRB0_N */
	err = ci20_pin_config(dev, 0, 20, PIN_GPIO_IN | PIN_PULL_DISABLE);
	if (err)
		return err;

	/* disable write protect */
	err = ci20_pin_config(dev, 5, 22, PIN_GPIO_OUT_HIGH);
	if (err)
		return err;

	/* ungate clocks */
	clearj_cgu_clkgr0(dev, CGU_CLKGR0_NEMC, env);

	/* timing */
	writej_nemc_smcr1(dev, 0x18664400, env);

	/* enable NAND flash */
	setj_nemc_nfcsr(dev, NEMC_NFCSR_NFE1, env);

	/* disable pseudo-random noise */
	writej_nemc_pncr(dev, 0x0, env);

	/* reset the NAND flash */
	writej_nand_cmd(dev, 0xff, env);
	setj_nemc_nfcsr(dev, NEMC_NFCSR_NFCE1, env);
	clearj_nemc_nfcsr(dev, NEMC_NFCSR_NFCE1, env);
	writej_nand_cmd(dev, 0x70, env);
	setj_nemc_nfcsr(dev, NEMC_NFCSR_NFCE1, env);

	/* wait until the flash is ready */
	do {
		status = readj_nand_data(dev, env);
	} while (!(status & 0x40));

	return 0;
}

int ci20_nand_id(struct ci20_dev *dev, unsigned char id[static 6])
{
	jmp_buf env;
	int err;

	err = setjmp(env);
	if (err)
		return err;

	writej_nand_cmd(dev, 0x90, env);
	writej_nand_addr(dev, 0, env);

	err = wait_ready(dev);
	if (err)
		return err;

	err = read_buf(dev, id, 6);
	if (err)
		return err;

	return 0;
}
