#
# ci20-tools
# Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

local_out := $(out)/usb-boot

source_files := \
	main.c

source := $(addprefix usb-boot/,$(source_files))
objs := $(addprefix $(local_out)/,$(patsubst %.c,%.o,$(source_files)))
-include $(objs:.o=.o.d)

libs := \
	-L$(dir $(ci20_lib)) -lci20

all: $(local_out)/ci20-usb-boot
$(local_out)/ci20-usb-boot: objs := $(objs)
$(local_out)/ci20-usb-boot: libs := $(libs)
$(local_out)/ci20-usb-boot: $(objs) $(ci20_lib) $(common_lib)
	@mkdir -p $(dir $@)
	@echo "CCLD    $@"
	$(SILENT)$(CC) -o $@ $(objs) $(common_lib) $(LDFLAGS)  $(libs)

$(objs): $$(patsubst $(local_out)/%.o,usb-boot/%.c,$$@) Makefile usb-boot/Makefile
	@mkdir -p $(dir $@)
	@echo "CC      $@"
	$(SILENT)$(CC) $(CFLAGS) -c $< -o $@
	$(SILENT)$(CC) $(CFLAGS) -MM -c $< -o - | \
		sed -e 's|^.*:|$@:|' >$@.d

.PHONY: install-usb-boot
install-tools: install-usb-boot
install-usb-boot: local_out := $(local_out)
install-usb-boot:
	install -Dm755 $(local_out)/ci20-usb-boot $(DESTDIR)/bin/ci20-usb-boot
