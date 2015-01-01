#
# ci20-tools
# Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

.SECONDEXPANSION:

CC := gcc
CFLAGS := -Wall -Werror -O2 -I.
LDFLAGS :=

CROSS_COMPILE ?= mips-linux-gnu-

CC_mips := $(CROSS_COMPILE)gcc
objcopy_mips := $(CROSS_COMPILE)objcopy

DESTDIR ?= /usr/local

V ?= 0
ifeq ($(V),1)
SILENT :=
else
SILENT := @
endif

out := out

.PHONY: all
all:

.PHONY: clean
clean:
	rm -rf $(out)

.PHONY: install
install: install-tools

.PHONY: install-tools
install-tools: all

include fw/Makefile
include lib/Makefile
include usb-test/Makefile
