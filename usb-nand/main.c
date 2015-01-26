/*
 * ci20-tools
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/ci20.h"
#include "lib/util.h"

#include "usb-common/discover.h"

static const unsigned cpu_hz = 1200000000;
static const unsigned extal_hz = 48000000;

static struct nand_info {
	uint16_t id;
	uint32_t ext_id;
	char *name;
} nand_chips[] = {
	{
		0x2c68, 0x00a94a04, "Micron MT29F32G08CBACA",
	},
	{
		0x2c88, 0x00a94b04, "Micron MT29F64G08CBAAA",
	},
};

static void usage(FILE *f)
{
	fprintf(f, "Usage: ci20-usb-nand <options>\n");
	fprintf(f, "\n");
	fprintf(f, "Where options is some combination of:\n");
	fprintf(f, "\n");
	fprintf(f, "  --help                 Display this message and exit\n");
	fprintf(f, "  --serial=<sn>          Use the board with this serial number\n");
	fprintf(f, "  --wait                 Wait if the board is not present\n");
}

int main(int argc, char *argv[])
{
	struct nand_info *nand;
	unsigned i;
	int err, ch, serial = -1;
	bool wait = false;
	unsigned char id_bytes[6];
	uint16_t id;
	uint32_t ext_id;
	struct option options[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "serial", required_argument, NULL, 's' },
		{ "wait", no_argument, NULL, 'w' },
		{ NULL, 0, NULL, 0 },
	};

	do {
		ch = getopt_long(argc, argv, "", options, NULL);

		switch (ch) {
		case 'h':
			usage(stdout);
			return EXIT_SUCCESS;

		case 's':
			serial = atoi(optarg);
			break;

		case 'w':
			wait = true;
			break;

		case '?':
		case ':':
			usage(stderr);
			return EXIT_FAILURE;
		}
	} while (ch != -1);

	err = common_init(serial, wait);
	if (err)
		return err;

	printf("Board Serial:     %" PRIu32 "\n", otp.serial);

	err = ci20_pll_init(dev, PLL_M, cpu_hz / extal_hz * 2, 2, 1);
	if (err) {
		printf("Failed to initialise PLL M\n");
		return EXIT_FAILURE;
	}

	err = ci20_mux_cpu_clk(dev, CPU_CLK_PLL_M);
	if (err) {
		printf("Failed to mux CPU clock\n");
		return EXIT_FAILURE;
	}

	err = ci20_nand_init(dev);
	if (err) {
		fprintf(stderr, "Failed to initialise NAND\n");
		return err;
	}

	err = ci20_nand_id(dev, id_bytes);
	if (err) {
		fprintf(stderr, "Failed to read NAND ID\n");
		return err;
	}

	id = ((uint16_t)id_bytes[0] << 8) | id_bytes[1];
	ext_id = ((uint32_t)id_bytes[5] << 24);
	ext_id |= ((uint32_t)id_bytes[4] << 16);
	ext_id |= ((uint32_t)id_bytes[3] << 8);
	ext_id |= id_bytes[2];

	for (i = 0; i < ARRAY_SIZE(nand_chips); i++) {
		if (nand_chips[i].id != id)
			continue;
		if (nand_chips[i].ext_id != ext_id)
			continue;

		nand = &nand_chips[i];
		break;
	}
	if (i >= ARRAY_SIZE(nand_chips)) {
		fprintf(stderr, "Unrecognised NAND flash 0x%04x 0x%08x\n", id, ext_id);
		return EXIT_FAILURE;
	}

	printf("NAND Flash  :     %s\n", nand->name);

	ci20_fini(ctx);
	return EXIT_SUCCESS;
}
