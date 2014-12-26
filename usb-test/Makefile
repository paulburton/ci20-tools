#
# ci20-tools
# Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

local_out := $(out)/usb-test

source_files := \
	main.c

source := $(addprefix usb-test/,$(source_files))
objs := $(addprefix $(local_out)/,$(patsubst %.c,%.o,$(source_files)))

libs := \
	-L$(dir $(ci20_lib)) -lci20

all: $(local_out)/ci20-usb-test
$(local_out)/ci20-usb-test: objs := $(objs)
$(local_out)/ci20-usb-test: libs := $(libs)
$(local_out)/ci20-usb-test: $(objs) $(ci20_lib)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(libs) -o $@ $(objs)

$(objs): $$(patsubst $(local_out)/%.o,usb-test/%.c,$$@) Makefile usb-test/Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: install-usb-test
install: install-usb-test
install-usb-test: local_out := $(local_out)
install-usb-test:
	install -Dm755 $(local_out)/ci20-usb-test $(DESTDIR)/bin/ci20-usb-test
