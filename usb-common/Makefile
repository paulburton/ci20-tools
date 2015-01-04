#
# ci20-tools
# Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

local_out := $(out)/usb-common

source_files := \
	discover.c

source := $(addprefix usb-common/,$(source_files))
objs := $(addprefix $(local_out)/,$(patsubst %.c,%.o,$(source_files)))
-include $(objs:.o=.o.d)

common_lib := $(local_out)/libcommon.a

$(common_lib): objs := $(objs)
$(common_lib): $(objs)
	@mkdir -p $(dir $@)
	@echo "AR      $@"
	$(SILENT)$(AR) rcsD $@ $(objs)

$(objs): $$(patsubst $(local_out)/%.o,usb-common/%.c,$$@) Makefile usb-common/Makefile
	@mkdir -p $(dir $@)
	@echo "CC      $@"
	$(SILENT)$(CC) $(CFLAGS) -c $< -o $@
	$(SILENT)$(CC) $(CFLAGS) -MM -c $< -o - | \
		sed -e 's|^.*:|$@:|' >$@.d
