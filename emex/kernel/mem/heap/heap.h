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

#ifndef HEAP_H
#define HEAP_H

#include <types.h>

typedef struct heap_block {
    u32 magic;
    u64 size;
    struct heap_block* next;
    struct heap_block* prev;
    u8 used;
} heap_block_t;

u64 *malloc(heap_block_t *heap, u64 size);
int free(u64 *ptr);

#define BLOCK_MAGIC 0xDEADBEEF

#endif
