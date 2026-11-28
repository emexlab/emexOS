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

#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/pmap.h>
#include <arch/emex64/page.h>
#include <arch/emex64/mmu.h>
#include <eboot_api.h>
#include <stdbool.h>

#define EMEX64_KHEAP_BASE       0x0010000000000000ULL
#define EMEX64_KHEAP_LIMIT      0x001FFFFFFFF00000ULL
#define KHEAP_GROW_PAGES        4ULL
#define KHEAP_ALIGN             16ULL
#define KHEAP_MAGIC             0x4B48454150424C4BULL

#define KHEAP_FLAG_FREE         (1U << 0)
#define KHEAP_FLAG_LARGE        (1U << 1)

typedef struct __attribute__((aligned(16))) kheap_block {
    uint64_t magic;
    size_t size;
    uint32_t flags;
    uint32_t _pad[3];
    struct kheap_block *prev;
    struct kheap_block *next;
} kheap_block_t;

_Static_assert(sizeof(kheap_block_t) == 48, "kheap_block_t must be 48 bytes");
_Static_assert((sizeof(kheap_block_t) % KHEAP_ALIGN) == 0, "kheap_block_t must stay 16-byte aligned");

static uint64_t kheap_arena_cursor = EMEX64_KHEAP_BASE;
static uint64_t kheap_large_cursor = 0x0018000000000000ULL;
static kheap_block_t *kheap_head = NULL;

static inline size_t kheap_align_up(size_t val)
{
    return (val + (KHEAP_ALIGN - 1)) & ~(KHEAP_ALIGN - 1);
}

static bool kheap_map_span(uint64_t vbase,
                           size_t page_count)
{
    for(size_t i = 0; i < page_count; i++)
    {
        void *phys = pmm_alloc(EMEX64_PAGE_SIZE);
        if(phys == NULL)
        {
            for(size_t j = 0; j < i; j++)
            {
                uint64_t vaddr = vbase + (j * EMEX64_PAGE_SIZE);
                uint64_t paddr = 0;
                if(pmap_extract(kernel_pmap, vaddr, &paddr))
                {
                    pmap_remove(kernel_pmap, vaddr, vaddr + EMEX64_PAGE_SIZE);
                    pmm_free((void *)paddr);
                }
            }
            return false;
        }

        uint64_t vaddr = vbase + (i * EMEX64_PAGE_SIZE);
        if(pmap_enter(kernel_pmap, vaddr, (uint64_t)phys, VM_PROT_READ | VM_PROT_WRITE, PMAP_WIRED) != 0)
        {
            pmm_free(phys);
            for(size_t j = 0; j < i; j++)
            {
                uint64_t rollback_va = vbase + (j * EMEX64_PAGE_SIZE);
                uint64_t rollback_pa = 0;
                if(pmap_extract(kernel_pmap, rollback_va, &rollback_pa))
                {
                    pmap_remove(kernel_pmap, rollback_va, rollback_va + EMEX64_PAGE_SIZE);
                    pmm_free((void *)rollback_pa);
                }
            }
            return false;
        }
    }

    return true;
}

static void kheap_unmap_span(uint64_t vbase,
                             size_t page_count)
{
    for(size_t i = 0; i < page_count; i++)
    {
        uint64_t vaddr = vbase + (i * EMEX64_PAGE_SIZE);
        uint64_t paddr = 0;
        if(pmap_extract(kernel_pmap, vaddr, &paddr))
        {
            pmap_remove(kernel_pmap, vaddr, vaddr + EMEX64_PAGE_SIZE);
            pmm_free((void *)paddr);
        }
    }
}

static void kheap_coalesce(kheap_block_t *blk)
{
    if(blk->next != NULL && (blk->next->flags & KHEAP_FLAG_FREE) != 0)
    {
        uint8_t *blk_end = (uint8_t *)blk + sizeof(kheap_block_t) + blk->size;
        if(blk_end == (uint8_t *)blk->next)
        {
            kheap_block_t *victim = blk->next;
            blk->size += sizeof(kheap_block_t) + victim->size;
            blk->next = victim->next;
            if(victim->next != NULL)
            {
                victim->next->prev = blk;
            }
            victim->magic = 0;
        }
    }
    if(blk->prev != NULL && (blk->prev->flags & KHEAP_FLAG_FREE) != 0)
    {
        uint8_t *prev_end = (uint8_t *)blk->prev + sizeof(kheap_block_t) + blk->prev->size;
        if(prev_end == (uint8_t *)blk)
        {
            kheap_block_t *prev = blk->prev;
            prev->size += sizeof(kheap_block_t) + blk->size;
            prev->next = blk->next;
            if(blk->next != NULL)
            {
                blk->next->prev = prev;
            }
            blk->magic = 0;
        }
    }
}

static kheap_block_t *kheap_grow(size_t min_bytes)
{
    size_t total_bytes = EMEX64_PAGE_ROUND_UP(min_bytes + sizeof(kheap_block_t));
    size_t pages = total_bytes / EMEX64_PAGE_SIZE;
    if(pages < KHEAP_GROW_PAGES)
    {
        pages = KHEAP_GROW_PAGES;
        total_bytes = pages * EMEX64_PAGE_SIZE;
    }

    if(kheap_arena_cursor + total_bytes > 0x0018000000000000ULL)
    {
        return NULL;
    }

    uint64_t vbase = kheap_arena_cursor;
    if(!kheap_map_span(vbase, pages))
    {
        return NULL;
    }
    kheap_arena_cursor += total_bytes;

    kheap_block_t *blk = (kheap_block_t *)vbase;
    blk->magic = KHEAP_MAGIC;
    blk->size = total_bytes - sizeof(kheap_block_t);
    blk->flags = KHEAP_FLAG_FREE;
    blk->prev = NULL;
    blk->next = NULL;

    if(kheap_head == NULL)
    {
        kheap_head = blk;
        return blk;
    }

    kheap_block_t *tail = kheap_head;
    while(tail->next != NULL)
    {
        tail = tail->next;
    }

    tail->next = blk;
    blk->prev = tail;
    kheap_coalesce(blk);

    return (blk->magic == KHEAP_MAGIC) ? blk : tail;
}

void kmalloc_init(void)
{
    kheap_grow(KHEAP_GROW_PAGES * EMEX64_PAGE_SIZE);
}

void *kmalloc(size_t size)
{
    if(size == 0)
    {
        return NULL;
    }

    size_t aligned_size = kheap_align_up(size);

    if(aligned_size > EMEX64_PAGE_SIZE)
    {
        size_t span_bytes = EMEX64_PAGE_ROUND_UP(aligned_size + sizeof(kheap_block_t));
        size_t pages = span_bytes / EMEX64_PAGE_SIZE;

        if(kheap_large_cursor + span_bytes + EMEX64_PAGE_SIZE > EMEX64_KHEAP_LIMIT)
        {
            return NULL;
        }

        uint64_t vbase = kheap_large_cursor;
        if(!kheap_map_span(vbase, pages))
        {
            return NULL;
        }

        kheap_large_cursor += span_bytes + EMEX64_PAGE_SIZE;

        kheap_block_t *blk = (kheap_block_t *)vbase;
        blk->magic = KHEAP_MAGIC;
        blk->size = span_bytes - sizeof(kheap_block_t);
        blk->flags = KHEAP_FLAG_LARGE;
        blk->prev = NULL;
        blk->next = NULL;

        return (void *)((uint8_t *)blk + sizeof(kheap_block_t));
    }

    kheap_block_t *curr = kheap_head;
    while(curr != NULL)
    {
        if((curr->flags & KHEAP_FLAG_FREE) != 0 && curr->size >= aligned_size)
        {
            break;
        }
        curr = curr->next;
    }

    if(curr == NULL)
    {
        curr = kheap_grow(aligned_size);
        if(curr == NULL)
        {
            return NULL;
        }
    }

    if(curr->size >= aligned_size + sizeof(kheap_block_t) + KHEAP_ALIGN)
    {
        kheap_block_t *split = (kheap_block_t *)((uint8_t *)curr + sizeof(kheap_block_t) + aligned_size);
        split->magic = KHEAP_MAGIC;
        split->size = curr->size - aligned_size - sizeof(kheap_block_t);
        split->flags = KHEAP_FLAG_FREE;
        split->prev = curr;
        split->next = curr->next;

        if(curr->next != NULL)
        {
            curr->next->prev = split;
        }
        curr->next = split;
        curr->size = aligned_size;
    }

    curr->flags &= ~KHEAP_FLAG_FREE;
    return (void *)((uint8_t *)curr + sizeof(kheap_block_t));
}

void *kcalloc(size_t nmemb, size_t size)
{
    if(nmemb == 0 || size == 0)
    {
        return NULL;
    }

    if(nmemb > (SIZE_MAX / size))
    {
        return NULL;
    }

    size_t total = nmemb * size;
    void *ptr = kmalloc(total);
    if(ptr != NULL)
    {
        memset(ptr, 0, total);
    }
    return ptr;
}

void *krealloc(void *ptr, size_t new_size)
{
    if(ptr == NULL)
    {
        return kmalloc(new_size);
    }

    if(new_size == 0)
    {
        kfree(ptr);
        return NULL;
    }

    kheap_block_t *blk = (kheap_block_t *)((uint8_t *)ptr - sizeof(kheap_block_t));
    if(blk->magic != KHEAP_MAGIC || (blk->flags & KHEAP_FLAG_FREE) != 0)
    {
        //kpanic("krealloc on invalid or freed block @ %p", ptr);
    }

    if(blk->size >= new_size)
    {
        return ptr;
    }

    void *next_ptr = kmalloc(new_size);
    if(next_ptr == NULL)
    {
        return NULL;
    }

    memcpy(next_ptr, ptr, blk->size);
    kfree(ptr);
    return next_ptr;
}

void kfree(void *ptr)
{
    if(ptr == NULL)
    {
        return;
    }

    kheap_block_t *blk = (kheap_block_t *)((uint8_t *)ptr - sizeof(kheap_block_t));
    if(blk->magic != KHEAP_MAGIC)
    {
        //kpanic("kfree on corrupted or non-heap pointer @ %p", ptr);
    }

    if((blk->flags & KHEAP_FLAG_FREE) != 0)
    {
        //kpanic("double kfree detected @ %p", ptr);
    }

    if((blk->flags & KHEAP_FLAG_LARGE) != 0)
    {
        size_t span_bytes = blk->size + sizeof(kheap_block_t);
        size_t pages = span_bytes / EMEX64_PAGE_SIZE;
        blk->magic = 0;
        kheap_unmap_span((uint64_t)blk, pages);
        return;
    }

    blk->flags |= KHEAP_FLAG_FREE;
    kheap_coalesce(blk);
}
