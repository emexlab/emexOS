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

#ifndef PHYSMEM_H
#define PHYSMEM_H

#include <types.h>
#include "../mem.h"

typedef struct physmem_pageframe {
    u32 rc;
    u32 flags;
} physmem_pageframe_t;

u64 physmem_free_get(void);
u64 physmem_alloc_to(u64 count);
void physmem_free_to(u64 physmem_addr, u64 count);
void physmem_init(limine_memmap_response_t *mpr, limine_hhdm_response_t *hpr);
u64 physmem_get_total();

#endif
