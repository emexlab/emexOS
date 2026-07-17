/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * emexOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * emexOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with emexOS. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PAGGING_H
#define PAGGING_H

#include <types.h>
#include "../mem.h"

//#define PAGE_SIZE 4096
#define PTE_PRESENT    (1ULL << 0)
#define PTE_WRITABLE   (1ULL << 1)
#define PTE_USER       (1ULL << 2)
#define PTE_PWT        (1ULL << 3)
#define PTE_PCD        (1ULL << 4)
#define PTE_ACCESSED   (1ULL << 5)
#define PTE_DIRTY      (1ULL << 6)
#define PTE_HUGE       (1ULL << 7)
#define PTE_GLOBAL     (1ULL << 8)

#define PTE_NO_EXEC    (1ULL << 63)
#define PTE_NX         (1ULL << 63)


#define KERNEL_FLAGS (PTE_PRESENT | PTE_WRITABLE)
#define USER_FLAGS (PTE_PRESENT | PTE_WRITABLE | PTE_USER)

typedef struct {
    u64 entries[512];
} page_table_t;

extern page_table_t *kernel_pml4;
extern u64 g_hhdm_offset;

void paging_map_page(limine_hhdm_response_t *hpr, u64 virtual_addr, u64 physical_addr, u64 flags);
void paging_unmap_page(u64 virtual_addr);
void paging_init(limine_hhdm_response_t *hpr);
u64 map_region_alloc(limine_hhdm_response_t *hpr, u64 virt, u64 size);
void map_region(limine_hhdm_response_t *hpr, u64 phys, u64 virt, u64 size, u64 flags);
void map_ulime_region(limine_hhdm_response_t *hpr, u64 phys_start, u64 size);
int verify_page_permissions(limine_hhdm_response_t *hpr, u64 virtual_addr, const char *name);

void* phys_to_virt(limine_hhdm_response_t *hpr, u64 phys_addr);
u64 virt_to_phys(limine_hhdm_response_t *hpr, void* virt_addr);

// perprocess page table support
u64  paging_create_proc_pml4 (limine_hhdm_response_t *hpr);
void paging_map_page_proc (limine_hhdm_response_t *hpr, u64 pml4_phys, u64 virtual_addr, u64 physical_addr, u64 flags);

#endif
