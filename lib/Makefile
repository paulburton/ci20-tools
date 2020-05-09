#
# libci20
# Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

local_out := $(out)/lib

source_files := \
	ci20.c \
	ddr.c \
	elf.c \
	nand.c \
	pll.c \
	usb.c

source := $(addprefix lib/,$(source_files))
objs := $(addprefix $(local_out)/,$(patsubst %.c,%.o,$(source_files)))
-include $(objs:.o=.o.d)

ci20_lib := $(local_out)/libci20.so

flags := \
	$(shell pkg-config --cflags libusb-1.0)

libs := \
	$(shell pkg-config --libs libusb-1.0)

all: $(ci20_lib)
$(ci20_lib): objs := $(objs)
$(ci20_lib): libs := $(libs)
$(ci20_lib): $(objs) $(fw_bin)
	@mkdir -p $(dir $@)
	@echo "CCLD    $@"
	$(SILENT)$(CC) -shared -o $@ $(objs) -Wl,--format=binary $(fw_bin) -Wl,--format=default $(LDFLAGS) $(libs)

$(objs): flags := $(flags)
$(objs): $$(patsubst $(local_out)/%.o,lib/%.c,$$@) Makefile lib/Makefile
	@mkdir -p $(dir $@)
	@echo "CC      $@"
	$(SILENT)$(CC) $(CFLAGS) $(flags) -fPIC -c $< -o $@
	$(SILENT)$(CC) $(CFLAGS) $(flags) -fPIC -MM -c $< -o - | \
		sed -e 's|^.*:|$@:|' >$@.d

.PHONY: install-lib
install: install-lib
install-lib:
	install -Dm755 $(ci20_lib) $(DESTDIR)/lib/$(notdir $(ci20_lib))
