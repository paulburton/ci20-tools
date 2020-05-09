#
# ci20-tools
# Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

local_out := $(out)/usb-nand

source_files := \
	main.c

source := $(addprefix usb-nand/,$(source_files))
objs := $(addprefix $(local_out)/,$(patsubst %.c,%.o,$(source_files)))
-include $(objs:.o=.o.d)

libs := \
	-L$(dir $(ci20_lib)) -lci20

all: $(local_out)/ci20-usb-nand
$(local_out)/ci20-usb-nand: objs := $(objs)
$(local_out)/ci20-usb-nand: libs := $(libs)
$(local_out)/ci20-usb-nand: $(objs) $(ci20_lib) $(common_lib)
	@mkdir -p $(dir $@)
	@echo "CCLD    $@"
	$(SILENT)$(CC) -o $@ $(objs) $(common_lib) $(LDFLAGS) $(libs)

$(objs): $$(patsubst $(local_out)/%.o,usb-nand/%.c,$$@) Makefile usb-nand/Makefile
	@mkdir -p $(dir $@)
	@echo "CC      $@"
	$(SILENT)$(CC) $(CFLAGS) -c $< -o $@
	$(SILENT)$(CC) $(CFLAGS) -MM -c $< -o - | \
		sed -e 's|^.*:|$@:|' >$@.d

.PHONY: install-usb-nand
install-tools: install-usb-nand
install-usb-nand: local_out := $(local_out)
install-usb-nand:
	install -Dm755 $(local_out)/ci20-usb-nand $(DESTDIR)/bin/ci20-usb-nand
