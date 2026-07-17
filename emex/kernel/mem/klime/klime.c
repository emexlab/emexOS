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

#include "klime.h"
#include "../heap/heap.h"
#include "../slab/slab.h"
#include "../paging/paging.h"
#include <kernel/arch/x86_64/exceptions/panic.h>

#include <kernel/mem/lib/main.h>
#include <kernel/communication/serial.h>

klime_t *klime_init(u64 *ptr, u64 size) {
    if (!ptr || size < sizeof(heap_block_t)) {
        panic("ERROR: Invalid klime initializing");
    }

    klime_t *klime = (klime_t *)ptr;

    klime->start_slab_meta = (u64 *)((u8 *)klime + KLIME_OFFSET_SLAB_META);
    klime->start_slab_data = (u64 *)((u8 *)klime + KLIME_OFFSET_SLAB_DATA);
    klime->start_io        = (u64 *)((u8 *)klime + KLIME_OFFSET_IO);
    klime->start_dma       = (u64 *)((u8 *)klime + KLIME_OFFSET_DMA);
    klime->start_heap      = (heap_block_t *)((u8 *)klime + KLIME_OFFSET_HEAP);
    klime->total_heap      = KLIME_SIZE_HEAP - sizeof(heap_block_t);
    klime->used_heap       = 0;

    klime->start_heap->magic = BLOCK_MAGIC;
    klime->start_heap->size = klime->total_heap;
    klime->start_heap->next = NULL;
    klime->start_heap->prev = NULL;
    klime->start_heap->used = 0;

    slab_init(&klime->slab_allocator, klime->start_slab_meta, klime->start_slab_data);

    return klime;
}

void klime_setup_slab(klime_t *klime) {
	(void)klime;
}

u64 *klime_create(klime_t *klime, u64 size) {
    if (!klime || size == 0) return NULL;

    u64 *ptr = malloc(klime->start_heap, size);
    if (!ptr) return NULL;

    heap_block_t *block = (heap_block_t*)((u8*)ptr - sizeof(heap_block_t));
    klime->used_heap += block->size + sizeof(heap_block_t);
    return ptr;
}

u64 *klime_alloc(klime_t *klime, u64 size, u64 count) {
    if (!klime || size == 0) return NULL;

    u64 total = count * size;
    u64 *ptr = malloc(klime->start_heap, total);
    if (!ptr) return NULL;
    memset(ptr, 0, total);

    heap_block_t *block = (heap_block_t*)((u8*)ptr - sizeof(heap_block_t));
    klime->used_heap += block->size + sizeof(heap_block_t);
    return ptr;
}

void klime_free(klime_t *klime, u64 *ptr) {
    if (!klime || !ptr) return;

    heap_block_t *block = (heap_block_t*)((u8*)ptr - sizeof(heap_block_t));
    klime->used_heap -= block->size;
    int merged = free(ptr);
    klime->used_heap -= sizeof(heap_block_t) * merged;
}

u64 klime_get_total_size(klime_t *klime) {
    return klime->total_heap;
}

u64 klime_get_used_size(klime_t *klime) {
    return klime->used_heap;
}

u64 klime_get_free_size(klime_t *klime) {
    return klime->total_heap - klime->used_heap;
}
