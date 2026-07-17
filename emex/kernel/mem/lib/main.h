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

#ifndef MAIN_H
#define MAIN_H

#include <types.h>

// BASICS
void memset(void *ptr, u8 val, size_t n);
void memcpy(void *dst, const void *src, size_t n);
void memmove(void *dst, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);

// Memory info is used in kernel but soon in console programms
u64 mem_get_free(void);
u64 mem_get_used(void);
u64 mem_get_total(void);

// DMA memory allocation (for disk I/O) (fat32)
void* kmalloc_dma(size_t size, size_t alignment);
void kfree_dma(void *ptr);

//page aligned allocation
void* kmalloc_aligned(size_t size, size_t alignment);

#endif
