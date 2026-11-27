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

#include <nvram.h>

uint8_t nvram_checksum(const uint8_t header[NVRAM_HEADER_SIZE])
{
    unsigned sum = header[0];
    for(int i = 2; i < NVRAM_HEADER_SIZE; i++)
    {
        sum += header[i];
        sum = (sum & 0xFF) + (sum >> 8);
    }
    return (uint8_t)sum;
}

static int is_common(const uint8_t *h)
{
    static const char name[12] = NVRAM_NAME_COMMON;
    if(h[0] != NVRAM_SIG_COMMON)
    {
        return 0;
    }
    for(int i = 0; i < 12; i++)
    {
        if(h[4 + i] != (uint8_t)name[i])
        {
            return 0;
        }
    }
    return 1;
}

int nvram_common(const uint8_t *nvram,
                 size_t size,
                 size_t *offset,
                 size_t *length)
{
    size_t at = 0;

    while(size - at >= NVRAM_HEADER_SIZE)
    {
        const uint8_t *h = nvram + at;
        size_t blocks = ((size_t)h[2] << 8) | h[3];
        size_t bytes = blocks * NVRAM_HEADER_SIZE;
        if(blocks == 0 || bytes > size - at || h[1] != nvram_checksum(h))
        {
            return -1;
        }
        if(is_common(h))
        {
            *offset = at + NVRAM_HEADER_SIZE;
            *length = bytes - NVRAM_HEADER_SIZE;
            return 0;
        }
        at += bytes;
    }
    return -1;
}

int nvram_get(const uint8_t *vars,
              size_t size,
              const char *name,
              const char **value,
              size_t *value_size)
{
    size_t at = 0;

    while(at < size && vars[at] != 0)
    {
        size_t end = at;
        while(end < size && vars[end] != 0)
        {
            end++;
        }

        size_t i = 0;
        while(name[i] != 0 && at + i < end && vars[at + i] == (uint8_t)name[i])
        {
            i++;
        }
        if(name[i] == 0 && at + i < end && vars[at + i] == '=')
        {
            *value = (const char *)vars + at + i + 1;
            *value_size = end - (at + i + 1);
            return 0;
        }
        at = end + 1;
    }
    return -1;
}
