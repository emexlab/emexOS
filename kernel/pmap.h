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

#ifndef SNU_PMAP_H
#define SNU_PMAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VM_PROT_NONE    0x0U
#define VM_PROT_READ    0x1U
#define VM_PROT_WRITE   0x2U
#define VM_PROT_EXECUTE 0x4U

#define PMAP_MMIO       0x1U
#define PMAP_WIRED      0x2U

typedef struct pmap *pmap_t;
extern pmap_t kernel_pmap;

void pmap_bootstrap(void);
pmap_t pmap_create(uint16_t asid);
void pmap_destroy(pmap_t pmap);
int pmap_enter(pmap_t pmap, uint64_t vaddr, uint64_t paddr, uint32_t prot, uint32_t flags);
void pmap_remove(pmap_t pmap, uint64_t start, uint64_t end);
bool pmap_extract(pmap_t pmap, uint64_t vaddr, uint64_t *paddr_out);
void pmap_activate(pmap_t pmap);

#endif /* SNU_PMAP_H */
