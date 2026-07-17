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

#ifndef KLIME_H
#define KLIME_H

#include "../mem.h"
#include <types.h>
#include "../slab/slab.h"
#include "../heap/heap.h"

typedef struct klime {
    u64 *start_slab_meta;
    u64 *start_slab_data;
    u64 *start_io;
    u64 *start_dma;
    heap_block_t *start_heap;
    slab_allocator_t slab_allocator;

    u64 total_heap;
    u64 used_heap;
} klime_t;

klime_t *klime_init(u64 *ptr, u64 size);

void klime_setup_slab(klime_t *klime);
void *klime_alloc_slab(klime_t *klime, u64 size);
void klime_free_slab(klime_t *klime, u64 *ptr);

u64 *klime_create(klime_t *klime, u64 size);

u64 *klime_alloc(klime_t *klime, u64 size, u64 count);
void klime_free(klime_t *klime, u64 *ptr);

u64  klime_get_total_size(klime_t *klime);
u64  klime_get_used_size(klime_t *klime);
u64  klime_get_free_size(klime_t *klime);

#endif
