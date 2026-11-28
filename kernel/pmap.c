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

#include <arch/emex64/mmio.h>
#include <arch/emex64/csr.h>
#include <arch/emex64/mmu.h>
#include <pmap.h>
#include <pmm.h>
#include <eboot_api.h>
#include <stdbool.h>

#define EMEX64_PT_LEVELS        4
#define EMEX64_PT_ENTRIES       1024ULL
#define EMEX64_PT_INDEX_MASK    0x3FFULL
#define EMEX64_VA_MAX           (1ULL << 53)

struct pmap
{
    uint64_t *root_table;
    uint16_t asid;
};

static struct pmap kernel_pmap_store;
pmap_t kernel_pmap = &kernel_pmap_store;

static const uint8_t pmap_level_shifts[EMEX64_PT_LEVELS] = { 43, 33, 23, 13 };

static uint64_t *pmap_alloc_table(void)
{
    uint64_t *table = (uint64_t *)pmm_alloc(EMEX64_PAGE_SIZE);
    if(table != NULL)
    {
        memset(table, 0, EMEX64_PAGE_SIZE);
    }
    return table;
}

static inline uint64_t pmap_encode_pfn(uint64_t paddr)
{
    return ((paddr >> 13) << 8) & EMEX64_MMU_PTB_PFN_MASK;
}

static inline uint64_t pmap_decode_pfn(uint64_t entry)
{
    return ((entry & EMEX64_MMU_PTB_PFN_MASK) >> 8) << 13;
}

static inline uint64_t pmap_encode_flags(pmap_t pmap,
                                         uint32_t prot,
                                         uint32_t flags)
{
    uint64_t pte_flags = EMEX64_MMU_PT_PRESENT;

    if((prot & VM_PROT_READ) != 0)
    {
        pte_flags |= EMEX64_MMU_PT_READ;
    }
    if((prot & VM_PROT_WRITE) != 0)
    {
        pte_flags |= EMEX64_MMU_PT_WRITE;
    }
    if((prot & VM_PROT_EXECUTE) != 0)
    {
        pte_flags |= EMEX64_MMU_PT_EXEC;
    }
    if((flags & PMAP_MMIO) != 0)
    {
        pte_flags |= EMEX64_MMU_PT_MMIO;
    }
    if(pmap != kernel_pmap)
    {
        pte_flags |= EMEX64_MMU_PT_USER;
    }

    return pte_flags;
}

static uint64_t *pmap_walk(pmap_t pmap,
                           uint64_t vaddr,
                           bool create)
{
    uint64_t *table = pmap->root_table;

    for(int level = 0; level < EMEX64_PT_LEVELS - 1; level++)
    {
        uint64_t idx = (vaddr >> pmap_level_shifts[level]) & EMEX64_PT_INDEX_MASK;
        uint64_t entry = table[idx];

        if((entry & EMEX64_MMU_PT_PRESENT) == 0)
        {
            if(!create)
            {
                return NULL;
            }

            uint64_t *next_table = pmap_alloc_table();
            if(next_table == NULL)
            {
                return NULL;
            }

            entry = pmap_encode_pfn((uint64_t)next_table) | EMEX64_MMU_PT_PRESENT;
            table[idx] = entry;
        }

        table = (uint64_t *)pmap_decode_pfn(entry);
    }

    uint64_t leaf_idx = (vaddr >> pmap_level_shifts[3]) & EMEX64_PT_INDEX_MASK;
    return &table[leaf_idx];
}

int pmap_enter(pmap_t pmap,
               uint64_t vaddr,
               uint64_t paddr,
               uint32_t prot,
               uint32_t flags)
{
    if(pmap == NULL || pmap->root_table == NULL || vaddr >= EMEX64_VA_MAX)
    {
        return -1;
    }

    vaddr = EMEX64_PAGE_ROUND_DOWN(vaddr);
    paddr = EMEX64_PAGE_ROUND_DOWN(paddr);

    if((flags & PMAP_MMIO) != 0 && paddr >= EMEX64_MMIO_BASE)
    {
        paddr -= EMEX64_MMIO_BASE;
    }

    if(paddr >= EMEX64_VA_MAX)
    {
        return -1;
    }

    uint64_t *pte = pmap_walk(pmap, vaddr, true);
    if(pte == NULL)
    {
        return -1;
    }

    *pte = pmap_encode_pfn(paddr) | pmap_encode_flags(pmap, prot, flags);
    return 0;
}

void pmap_remove(pmap_t pmap,
                 uint64_t start,
                 uint64_t end)
{
    if(pmap == NULL || pmap->root_table == NULL || end <= start)
    {
        return;
    }

    start = EMEX64_PAGE_ROUND_DOWN(start);
    if(end > EMEX64_VA_MAX)
    {
        end = EMEX64_VA_MAX;
    }

    for(uint64_t addr = start; addr < end; addr += EMEX64_PAGE_SIZE)
    {
        uint64_t *pte = pmap_walk(pmap, addr, false);
        if(pte != NULL && (*pte & EMEX64_MMU_PT_PRESENT) != 0)
        {
            *pte = 0;
        }
    }
}

void pmap_protect(pmap_t pmap,
                  uint64_t start,
                  uint64_t end,
                  uint32_t prot)
{
    if(pmap == NULL || pmap->root_table == NULL || end <= start)
    {
        return;
    }

    start = EMEX64_PAGE_ROUND_DOWN(start);
    if(end > EMEX64_VA_MAX)
    {
        end = EMEX64_VA_MAX;
    }

    for(uint64_t addr = start; addr < end; addr += EMEX64_PAGE_SIZE)
    {
        uint64_t *pte = pmap_walk(pmap, addr, false);
        if(pte != NULL && (*pte & EMEX64_MMU_PT_PRESENT) != 0)
        {
            if(prot == VM_PROT_NONE)
            {
                *pte = 0;
                continue;
            }

            uint32_t flags = ((*pte & EMEX64_MMU_PT_MMIO) != 0) ? PMAP_MMIO : 0;
            uint64_t pfn_bits = *pte & EMEX64_MMU_PTB_PFN_MASK;
            *pte = pfn_bits | pmap_encode_flags(pmap, prot, flags);
        }
    }
}

bool pmap_extract(pmap_t pmap,
                  uint64_t vaddr,
                  uint64_t *paddr_out)
{
    if(pmap == NULL || pmap->root_table == NULL || paddr_out == NULL || vaddr >= EMEX64_VA_MAX)
    {
        return false;
    }

    uint64_t *pte = pmap_walk(pmap, vaddr, false);
    if(pte == NULL || (*pte & EMEX64_MMU_PT_PRESENT) == 0)
    {
        return false;
    }

    uint64_t phys_base = pmap_decode_pfn(*pte);
    if((*pte & EMEX64_MMU_PT_MMIO) != 0)
    {
        phys_base += EMEX64_MMIO_BASE;
    }

    *paddr_out = phys_base + (vaddr & EMEX64_PAGE_MASK);
    return true;
}

pmap_t pmap_create(uint16_t asid)
{
    pmap_t pmap = (pmap_t)pmm_alloc(sizeof(struct pmap));
    if(pmap == NULL)
    {
        return NULL;
    }

    pmap->root_table = pmap_alloc_table();
    if(pmap->root_table == NULL)
    {
        pmm_free(pmap);
        return NULL;
    }

    pmap->asid = asid;
    return pmap;
}

static void pmap_free_level(uint64_t *table,
                            int level)
{
    if(table == NULL)
    {
        return;
    }

    if(level < EMEX64_PT_LEVELS - 1)
    {
        for(uint64_t i = 0; i < EMEX64_PT_ENTRIES; i++)
        {
            if((table[i] & EMEX64_MMU_PT_PRESENT) != 0)
            {
                uint64_t *child = (uint64_t *)pmap_decode_pfn(table[i]);
                pmap_free_level(child, level + 1);
            }
        }
    }

    pmm_free(table);
}

void pmap_destroy(pmap_t pmap)
{
    if(pmap == NULL || pmap == kernel_pmap)
    {
        return;
    }

    pmap_free_level(pmap->root_table, 0);
    pmm_free(pmap);
}

void pmap_activate(pmap_t pmap)
{
    cr_write(EMEX64_CREG_PTB, (((uint64_t)pmap->asid) << EMEX64_MMU_ASID_SHIFT) | pmap_encode_pfn((uint64_t)pmap->root_table) | EMEX64_MMU_PT_PRESENT);
}

void pmap_bootstrap(void)
{
    uint64_t phys_end = EMEX64_PAGE_ROUND_DOWN(mmio_read64(EMEX64_MC_REG_SIZE));

    kernel_pmap_store.root_table = pmap_alloc_table();
    kernel_pmap_store.asid = 0;

    for(uint64_t addr = 0; addr < phys_end; addr += EMEX64_PAGE_SIZE)
    {
        pmap_enter(kernel_pmap, addr, addr, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE, PMAP_WIRED);
    }

    for(uint64_t addr = EMEX64_MMIO_BASE; addr < EMEX64_MMIO_END; addr += EMEX64_PAGE_SIZE)
    {
        pmap_enter(kernel_pmap, addr, addr, VM_PROT_READ | VM_PROT_WRITE, PMAP_MMIO | PMAP_WIRED);
    }

    pmap_activate(kernel_pmap);
}
