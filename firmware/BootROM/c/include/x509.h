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

#ifndef X509_H
#define X509_H

#include <der.h>

#define X509_KEY_SIZE               32
#define X509_SIGNATURE_SIZE         64

#define X509_DIGITAL_SIGNATURE      (1 << 0)
#define X509_KEY_CERT_SIGN          (1 << 5)

struct x509 {
    const uint8_t *tbs;
    size_t tbs_size;
    const uint8_t *issuer;
    size_t issuer_size;
    const uint8_t *subject;
    size_t subject_size;
    const uint8_t *public_key;
    const uint8_t *signature;
    int is_ca;
    int path_length;
    int key_usage;
};

int x509_parse(const uint8_t *p, size_t size, struct x509 *cert);
int x509_check(const struct x509 *cert, const uint8_t issuer_key[X509_KEY_SIZE]);

#endif /* X509_H */
