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

#ifndef SLAB_H
#define SLAB_H

#include <types.h>

#define PAGE_SIZE 4096
#define SLAB_MIN_SIZE 16
#define SLAB_MAX_SIZE 1024
#define SLAB_CACHE_COUNT 7  // 16, 32, 64, 128, 256, 512, 1024 bytes

typedef struct slab_allocator {
    u64 sizes[SLAB_CACHE_COUNT];
} slab_allocator_t;

enum slab_result{
    InvalidPtrSlab,
    InvalidPtrSlabMeta,
    InvalidPtrSlabData,

    SlabInitSuccess,
};

void slab_init(slab_allocator_t *ptr_slab, u64 *ptr_slab_meta, u64 *ptr_slab_data);

#endif
