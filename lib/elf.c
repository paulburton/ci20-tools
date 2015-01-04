/*
 * libci20
 * Copyright (C) 2015 Paul Burton <paulburton89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ci20.h"

int ci20_load_elf(struct ci20_dev *dev, const void *base, uint32_t *entry)
{
	const Elf32_Ehdr *ehdr = base;
	const Elf32_Phdr *phdr;
	unsigned i;
	int err;

	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG))
		return -EINVAL;

	if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
		return -EINVAL;

	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
		return -EINVAL;

	if (ehdr->e_ident[EI_VERSION] != EV_CURRENT)
		return -EINVAL;

	if (ehdr->e_type != ET_EXEC)
		return -EINVAL;

	if (ehdr->e_machine != EM_MIPS)
		return -EINVAL;

	*entry = ehdr->e_entry;

	phdr = base + ehdr->e_phoff;
	for (i = 0; i < ehdr->e_phnum; i++, phdr++) {
		if (phdr->p_type != PT_LOAD)
			continue;

		err = ci20_writemem(dev, base + phdr->p_offset, phdr->p_filesz, phdr->p_vaddr);
		if (err)
			return err;

		if (phdr->p_memsz > phdr->p_filesz) {
			err = ci20_memset(dev, phdr->p_vaddr + phdr->p_filesz, 0,
					  phdr->p_memsz - phdr->p_filesz);
			if (err)
				return err;
		}

		err = ci20_dcache_flush(dev, phdr->p_vaddr, phdr->p_memsz);
		if (err)
			return err;
	}

	return 0;
}

int ci20_load_elf_fd(struct ci20_dev *dev, int filedes, uint32_t *entry)
{
	struct stat st;
	void *base;
	int err;

	if (fstat(filedes, &st))
		return errno;

	base = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, filedes, 0);
	if (base == MAP_FAILED)
		return errno;

	err = ci20_load_elf(dev, base, entry);
	munmap(base, st.st_size);
	return err;
}

int ci20_load_elf_path(struct ci20_dev *dev, const char *path, uint32_t *entry)
{
	int filedes, err;

	filedes = open(path, O_RDONLY);
	if (filedes == -1)
		return errno;

	err = ci20_load_elf_fd(dev, filedes, entry);
	close(filedes);
	return err;
}
