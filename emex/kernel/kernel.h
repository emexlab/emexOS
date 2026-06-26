/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: kernel.h
 * CREATED BY: mimalloc (Catelyn)
 * MODIFIED BY: --
 *
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <kernel/mem/meminclude.h>

#include "stdarg.h"
#include "types.h"

#define kmalloc(size) klime_alloc(kglobal.klime, size, 1)
#define kfree(ptr) klime_free(kglobal.klime, ptr)

typedef struct {
    klime_t *klime;
} kglobal_t;

extern kglobal_t kglobal;

static u64 *kcalloc(u64 size,
                    u64 count)
{
    u64 total = size * count;
    u64 *ptr = kmalloc(total);

    /* nullify memory like in calloc */
    u8 *mem = (u8*)ptr;
    for(u64 i = 0; i < total; i++)
    {
        mem[i] = 0;
    }

    return ptr;
}

#endif /* KERNEL_H */