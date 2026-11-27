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

#ifndef DER_H
#define DER_H

#include <stddef.h>
#include <stdint.h>

#define DER_UNIVERSAL       0
#define DER_APPLICATION     1
#define DER_CONTEXT         2
#define DER_PRIVATE         3

#define DER_BOOLEAN         1
#define DER_INTEGER         2
#define DER_BIT_STRING      3
#define DER_OCTET_STRING    4
#define DER_OID             6
#define DER_IA5_STRING      22
#define DER_SEQUENCE        16
#define DER_SET             17

struct der {
    uint8_t cls;
    uint8_t constructed;
    uint32_t tag;
    const uint8_t *element;
    size_t element_size;
    const uint8_t *content;
    size_t length;
};

struct der_list {
    const uint8_t *next;
    size_t left;
};

int der_read(const uint8_t *p, size_t size, struct der *out);
void der_open(const struct der *parent, struct der_list *list);
int der_next(struct der_list *list, struct der *out);
int der_expect(struct der_list *list, uint8_t cls, uint8_t constructed, uint32_t tag, struct der *out);
int der_end(const struct der_list *list);
int der_uint64(const struct der *e, uint64_t *value);
int der_bool(const struct der *e, int *value);
int der_is(const struct der *e, const void *bytes, size_t size);
int der_same(const void *a, const void *b, size_t size);

#endif /* DER_H */
