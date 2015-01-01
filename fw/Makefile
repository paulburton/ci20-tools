#
# libci20 Firmware
# Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

FW_DEBUG ?= 0

local_out := $(out)/fw

source_files := \
	main.c \
	uart.c \
	usb.c

source := $(addprefix fw/,$(source_files))
objs := $(addprefix $(local_out)/,$(patsubst %.c,%.o,$(source_files)))
-include $(local_out)/start.o.d
-include $(objs:.o=.o.d)

fw_bin := $(local_out)/fw.bin
fw_elf := $(local_out)/fw.elf

flags := -mips32r2 -EL -G0 -mno-gpopt -msoft-float -flto
flags += -ffreestanding -nostdlib -mno-abicalls
flags += -DDEBUG=$(FW_DEBUG) -ggdb

all: $(fw_bin)
$(fw_bin): $(fw_elf)
	@echo "OBJCOPY $@"
	$(SILENT)$(objcopy_mips) -O binary $< $@

$(fw_elf): start := $(local_out)/start.o
$(fw_elf): flags := $(flags)
$(fw_elf): objs := $(objs)
$(fw_elf): $(local_out)/start.o $(objs) fw/link.lds
	@mkdir -p $(dir $@)
	@echo "CCLD    $@"
	$(SILENT)$(CC_mips) $(LDFLAGS) $(flags) -T fw/link.lds -o $@ $(start) $(objs)

$(local_out)/start.o: flags := $(flags)
$(local_out)/start.o: fw/start.S Makefile fw/Makefile
	@mkdir -p $(dir $@)
	@echo "CC      $@"
	$(SILENT)$(CC_mips) $(CFLAGS) $(flags) -c $< -o $@
	$(SILENT)$(CC_mips) $(CFLAGS) $(flags) -MM -c $< -o - | \
		sed -e 's|^.*:|$@:|' >$@.d

$(objs): flags := $(flags)
$(objs): $$(patsubst $(local_out)/%.o,fw/%.c,$$@) Makefile fw/Makefile
	@mkdir -p $(dir $@)
	@echo "CC      $@"
	$(SILENT)$(CC_mips) $(CFLAGS) $(flags) -c $< -o $@
	$(SILENT)$(CC_mips) $(CFLAGS) $(flags) -MM -c $< -o - | \
		sed -e 's|^.*:|$@:|' >$@.d
