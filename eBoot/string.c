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

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dst,
             const void *src,
             size_t n)
{
    __asm__ __volatile__("cpym %0, %1, %2" : : "r"(dst), "r"(src), "r"(n) : "memory");
    return dst;
}

void *memmove(void *dst,
              const void *src,
              size_t n)
{
    __asm__ __volatile__("movm %0, %1, %2" : : "r"(dst), "r"(src), "r"(n) : "memory");
    return dst;
}

void *memset(void *dst,
             int value,
             size_t n)
{
    __asm__ __volatile__("setm %0, %1, %2" : : "r"(dst), "r"((uint64_t)value), "r"(n) : "memory");
    return dst;
}

int memcmp(const void *a,
           const void *b,
           size_t n)
{
    const uint8_t *x = a;
    const uint8_t *y = b;
    for(size_t i = 0; i < n; i++)
    {
        if(x[i] != y[i])
        {
            return x[i] < y[i] ? -1 : 1;
        }
    }
    return 0;
}
