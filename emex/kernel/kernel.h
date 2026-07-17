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
