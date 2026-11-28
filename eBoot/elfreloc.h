/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
 *
 * Private use and modification are permitted under the license.
 * Redistribution of source or binaries, modified or unmodified,
 * requires prior written permission from emexLabs.
 *
 * External deployment is subject to the source-availability
 * requirements and other conditions of the license.
 *
 * Provided "AS IS", without warranty, to the extent permitted by law.
 *
 * See LICENSE for the full license terms.
 */

#ifndef EBOOT_ELFRELOC_H
#define EBOOT_ELFRELOC_H

#include <stddef.h>
#include <stdint.h>
#include "export.h"

#define ELF_SYMTAB_MAX_IMAGES  16
#define ELF_SYMTAB_MAX_MANUAL  32

typedef struct {
    uint64_t base;
    uint64_t span;
    uint64_t load_bias;
    uint64_t entry;
    const void *dynsym;
    size_t dynsym_count;
    const char *dynstr;
    size_t dynstr_size;
} elf_image_t;

typedef struct {
    const char *name;
    uint64_t addr;
} elf_manual_sym_t;

typedef struct {
    elf_image_t images[ELF_SYMTAB_MAX_IMAGES];
    size_t image_count;
    elf_manual_sym_t manual[ELF_SYMTAB_MAX_MANUAL];
    size_t manual_count;
} elf_symtab_t;

EBOOT_EXPORT void elf_symtab_init(elf_symtab_t *tab);
EBOOT_EXPORT int elf_symtab_add_image(elf_symtab_t *tab, const elf_image_t *img);
EBOOT_EXPORT int elf_symtab_add_symbol(elf_symtab_t *tab, const char *name, uint64_t addr);
EBOOT_EXPORT int elf_symtab_lookup(const elf_symtab_t *tab, const char *name, uint64_t *addr_out);

EBOOT_EXPORT const char *elfreloc_load_ex(const uint8_t *elf, size_t size, uint64_t *cursor, uint64_t limit, const elf_symtab_t *imports, elf_image_t *image_out, const char **missing_sym_out);
EBOOT_EXPORT const char *elfreloc_load(const uint8_t *elf, size_t size, uint64_t *cursor, uint64_t limit, uint64_t *entry_out);

#endif /* EBOOT_ELFRELOC_H */