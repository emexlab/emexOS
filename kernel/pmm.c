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

#include <pmm.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/emex64/mmio.h>
#include <arch/emex64/random.h>
#include <eboot_api.h>

#define PAGE_FREE 0x0
#define PAGE_HEAD 0x1
#define PAGE_CONT 0x2

#ifndef PMM_DEBUG_ENABLED
#define PMM_DEBUG_ENABLED 0
#endif /* PMM_DEBUG_ENABLED */

/* eBoot shall boot us as the last image */
extern char __kernel_end[] __attribute__((visibility("hidden")));

static void *pmm_space_start = NULL;
static void *pmm_space_end = NULL;

static void *pfmr_region_start = NULL;
static void *pfmr_region_end = NULL;

static uint64_t total_alloc_pages = 0;

static inline uint8_t pmm_get_state(uint64_t page_idx)
{
    uint8_t *bitmap = (uint8_t *)pfmr_region_start;
    return (bitmap[page_idx / 4] >> ((page_idx % 4) * 2)) & 0x3;
}

static inline void pmm_set_state(uint64_t page_idx,
                                 uint8_t state)
{
    uint8_t *bitmap = (uint8_t *)pfmr_region_start;
    uint64_t byte_idx = page_idx / 4;
    uint64_t shift = (page_idx % 4) * 2;
    bitmap[byte_idx] = (bitmap[byte_idx] & ~(0x3U << shift)) | ((state & 0x3U) << shift);
}

void pmm_init(void)
{
    printf("[PMM] computing offsets and sizes\n");

    /* computing bounds */
    pmm_space_start = (void*)EMEX64_PAGE_ROUND_UP((uint64_t)__kernel_end);
    uint64_t phys_end = mmio_read64(EMEX64_MC_REG_SIZE);
    pmm_space_end = (void*)EMEX64_PAGE_ROUND_DOWN(phys_end);

    uint64_t pmmr_size = ((uint64_t)pmm_space_end - (uint64_t)pmm_space_start);
    uint64_t pmmr_page_count = pmmr_size / EMEX64_PAGE_SIZE;
#if PMM_DEBUG_ENABLED
    printf("[PMM] page_memory_manager_region:  %p - %p\n",  pmm_space_start, pmm_space_end);
    printf("[PMM] page_memory_manager_region.page_count: %llu\n", pmmr_page_count);
#endif /* PMM_DEBUG_ENABLED */

    /* computing size needed for mark page */
    /* TODO: don't calculate PFMR into the region */
    uint64_t pfmr_size = EMEX64_PAGE_ROUND_UP((pmmr_page_count + 3) / 4);
    uint64_t pfmr_page_count = pfmr_size / EMEX64_PAGE_SIZE;
    pfmr_region_start = pmm_space_start;
    pfmr_region_end = (void *)((uint64_t)pmm_space_start + (EMEX64_PAGE_SIZE * pfmr_page_count));
    pmm_space_start = pfmr_region_end; /* so you cannot use this memory */

    total_alloc_pages = ((uint64_t)pmm_space_end - (uint64_t)pmm_space_start) / EMEX64_PAGE_SIZE;

    /* zero out the bitmap */
    uint8_t *bitmap = (uint8_t *)pfmr_region_start;
    memset(bitmap, 0, pfmr_size);

#if PMM_DEBUG_ENABLED
    printf("[PMM] page_frame_memory_region.size: %llu bytes\n", pfmr_size);
    printf("[PMM] page_frame_memory_region.page_count: %llu\n", pfmr_page_count);
#endif /* PMM_DEBUG_ENABLED */
    printf("[PMM] page_frame_memory_region: %p - %p\n", pfmr_region_start, pfmr_region_end);
    printf("[PMM] page_memory_manager_allocation_region: %p - %p\n", pmm_space_start, pmm_space_end);
}

void *pmm_alloc(size_t size)
{
    if(size == 0)
    {
        return NULL;
    }

    uint64_t needed_pages = EMEX64_PAGE_ROUND_UP(size) / EMEX64_PAGE_SIZE;
    uint64_t run_start = 0;
    uint64_t run_length = 0;

    for(uint64_t i = 0; i < total_alloc_pages; i++)
    {
        if(pmm_get_state(i) == PAGE_FREE)
        {
            if(run_length == 0)
            {
                run_start = i;
            }
            run_length++;

            if(run_length == needed_pages)
            {
                pmm_set_state(run_start, PAGE_HEAD);
                for(uint64_t j = 1; j < needed_pages; j++)
                {
                    pmm_set_state(run_start + j, PAGE_CONT);
                }
                return (void *)((uint64_t)pmm_space_start + (run_start * EMEX64_PAGE_SIZE));
            }
        }
        else
        {
            run_length = 0;
        }
    }

    /* oom */
    return NULL;
}

void pmm_free(void *ptr)
{
    if(ptr == NULL)
    {
        return;
    }

    uint64_t addr = (uint64_t)ptr;
    uint64_t start = (uint64_t)pmm_space_start;
    uint64_t end = (uint64_t)pmm_space_end;

    if(addr < start || addr >= end || ((addr - start) % EMEX64_PAGE_SIZE) != 0)
    {
        return;
    }

    uint64_t page_idx = (addr - start) / EMEX64_PAGE_SIZE;
    if(pmm_get_state(page_idx) != PAGE_HEAD)
    {
        return;
    }

    pmm_set_state(page_idx, PAGE_FREE);

    for(uint64_t i = page_idx + 1; i < total_alloc_pages; i++)
    {
        if(pmm_get_state(i) == PAGE_CONT)
        {
            pmm_set_state(i, PAGE_FREE);
        }
        else
        {
            break;
        }
    }
}
