/*
 * ci20-tools
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
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

#include "usb-common/discover.h"

static const unsigned cpu_hz = 1200000000;
static const unsigned mem_hz = 400000000;
static const unsigned extal_hz = 48000000;

static void usage(FILE *f)
{
	fprintf(f, "Usage: ci20-usb-boot <options> ELF\n");
	fprintf(f, "\n");
	fprintf(f, "Where options is some combination of:\n");
	fprintf(f, "\n");
	fprintf(f, "  --help                 Display this message and exit\n");
	fprintf(f, "  --serial=<sn>          Use the board with this serial number\n");
}

int main(int argc, char *argv[])
{
	uint32_t entry;
	int err, ch, serial = -1;
	const char *elf_path;
	struct option options[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "serial", required_argument, NULL, 's' },
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

		case '?':
		case ':':
			usage(stderr);
			return EXIT_FAILURE;
		}
	} while (ch != -1);

	if (optind >= argc) {
		usage(stderr);
		return EXIT_FAILURE;
	}

	elf_path = argv[optind];

	err = common_init(serial);
	if (err)
		return err;

	printf("Initializing PLL M...");
	err = ci20_pll_init(dev, PLL_M, cpu_hz / extal_hz * 2, 2, 1);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	printf("Muxing CPU clock...");
	err = ci20_mux_cpu_clk(dev, CPU_CLK_PLL_M);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	printf("Muxing DDR clock...");
	err = ci20_mux_ddr_clk(dev, DDR_CLK_PLL_M, cpu_hz / mem_hz);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	printf("Initializing DDR...");
	err = ci20_ddr_init(dev, mem_hz);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	printf("Initializing icache...");
	err = ci20_icache_init(dev);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	printf("Initializing dcache...");
	err = ci20_dcache_init(dev);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	printf("Enabling caches...");
	err = ci20_set_k0_cca(dev, 3);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	printf("Loading kernel...");
	err = ci20_load_elf_path(dev, elf_path, &entry);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	printf("Jumping to kernel at 0x%08" PRIx32 "...", entry);
	err = ci20_jump(dev, entry);
	if (err) {
		printf(" failed!\n");
		return EXIT_FAILURE;
	}
	printf(" done\n");

	ci20_fini(ctx);
	return EXIT_SUCCESS;
}
