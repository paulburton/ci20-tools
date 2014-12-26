/*
 * libci20 Firmware
 * Copyright (C) 2014 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

OUTPUT_FORMAT("elf32-tradbigmips", "elf32-tradbigmips", "elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(start)
SECTIONS {
	. = 0xf4000800;

	.text : {
		*(.text.start)
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { *(.data*) }
	.sdata : { *(.sdata*) }

	. = ALIGN(4);
	__bss_start = .;
	.sbss (NOLOAD) : { *(.sbss*) }
	.bss (NOLOAD) : { *(.bss*) }
	. = ALIGN(4);
	__bss_end = .;

	/DISCARD/ : {
		*(*)
	}
}
