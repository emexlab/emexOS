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

#include <der.h>

int der_read(const uint8_t *p,
             size_t size,
             struct der *out)
{
    size_t at = 0;
    uint32_t tag;

    if(size < 2)
    {
        return -1;
    }

    out->cls = p[0] >> 6;
    out->constructed = (p[0] >> 5) & 1;
    tag = p[0] & 0x1F;
    at = 1;

    if(tag == 0x1F)
    {
        tag = 0;
        if(p[at] == 0x80)
        {
            return -1;
        }
        for(;;)
        {
            if(at >= size || tag > (UINT32_MAX >> 7))
            {
                return -1;
            }
            uint8_t b = p[at++];
            tag = (tag << 7) | (b & 0x7F);
            if(!(b & 0x80))
            {
                break;
            }
        }
        if(tag < 0x1F)
        {
            return -1;
        }
    }
    out->tag = tag;

    if(at >= size)
    {
        return -1;
    }
    size_t length = p[at++];
    if(length & 0x80)
    {
        size_t bytes = length & 0x7F;
        if(bytes == 0 || bytes > sizeof(size_t) || bytes > size - at || p[at] == 0)
        {
            return -1;
        }
        length = 0;
        for(size_t i = 0; i < bytes; i++)
        {
            length = (length << 8) | p[at++];
        }
        if(length < 0x80)
        {
            return -1;
        }
    }
    if(length > size - at)
    {
        return -1;
    }

    out->element = p;
    out->content = p + at;
    out->length = length;
    out->element_size = at + length;
    return 0;
}

void der_open(const struct der *parent,
              struct der_list *list)
{
    list->next = parent->content;
    list->left = parent->length;
}

int der_next(struct der_list *list,
             struct der *out)
{
    if(list->left == 0)
    {
        return 0;
    }
    if(der_read(list->next, list->left, out) != 0)
    {
        return -1;
    }
    list->next += out->element_size;
    list->left -= out->element_size;
    return 1;
}

int der_expect(struct der_list *list,
               uint8_t cls,
               uint8_t constructed,
               uint32_t tag,
               struct der *out)
{
    if(der_next(list, out) != 1 || out->cls != cls || out->constructed != constructed || out->tag != tag)
    {
        return -1;
    }
    return 0;
}

int der_end(const struct der_list *list)
{
    return list->left == 0;
}

int der_uint64(const struct der *e,
               uint64_t *value)
{
    const uint8_t *p = e->content;
    size_t n = e->length;

    if(e->cls != DER_UNIVERSAL || e->constructed || e->tag != DER_INTEGER || n == 0 || (p[0] & 0x80))
    {
        return -1;
    }
    if(n > 1 && p[0] == 0 && !(p[1] & 0x80))
    {
        return -1;
    }
    if(p[0] == 0)
    {
        p++;
        n--;
    }
    if(n > 8)
    {
        return -1;
    }

    uint64_t v = 0;
    for(size_t i = 0; i < n; i++)
    {
        v = (v << 8) | p[i];
    }
    *value = v;
    return 0;
}

int der_bool(const struct der *e,
             int *value)
{
    if(e->cls != DER_UNIVERSAL || e->constructed || e->tag != DER_BOOLEAN || e->length != 1 ||
       (e->content[0] != 0x00 && e->content[0] != 0xFF))
    {
        return -1;
    }
    *value = e->content[0] == 0xFF;
    return 0;
}

int der_same(const void *a,
             const void *b,
             size_t size)
{
    const uint8_t *x = a;
    const uint8_t *y = b;
    for(size_t i = 0; i < size; i++)
    {
        if(x[i] != y[i])
        {
            return 0;
        }
    }
    return 1;
}

int der_is(const struct der *e,
           const void *bytes,
           size_t size)
{
    return e->length == size && der_same(e->content, bytes, size);
}
