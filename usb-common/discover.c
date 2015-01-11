/*
 * ci20-tools
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/ci20.h"

#include "discover.h"

struct ci20_ctx *ctx;
struct ci20_dev *dev;
struct ci20_otp otp;

static void discover_cb(struct ci20_ctx *c, struct ci20_dev *_dev, void *user)
{
	int serial = *(int *)user;
	int err;

	err = ci20_read_otp(_dev, &otp);
	if (err) {
		fprintf(stderr, "Failed to read OTP\n");
		return;
	}

	if ((serial != -1) && (serial != otp.serial)) {
		/* this is not the board you're looking for */
		return;
	}

	if (dev) {
		fprintf(stderr, "Multiple boards detected. Please specify --serial.\n");
		exit(EXIT_FAILURE);
	}

	dev = _dev;
}

int common_init(int serial, bool wait)
{
	ctx = ci20_init();
	if (!ctx) {
		fprintf(stderr, "Failed to initialise libci20\n");
		return EXIT_FAILURE;
	}

	do {
		ci20_discover(ctx, discover_cb, &serial);
	} while (!dev && wait);

	if (!dev) {
		fprintf(stderr, "CI20 not found\n");
		return EXIT_FAILURE;
	}

	return 0;
}
