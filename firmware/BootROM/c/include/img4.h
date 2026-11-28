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

#ifndef IMG4_H
#define IMG4_H

#include <x509.h>

#define IMG4_DIGEST_SIZE        64
#define IMG4_NONCE_HASH_SIZE    64
#define IMG4_MAX_CHAIN          4

#define IMG4_CHIP               0x2400
#define IMG4_BOARD              0

#define IMG4_FOURCC(s)          (((uint32_t)(uint8_t)(s)[0] << 24) | ((uint32_t)(uint8_t)(s)[1] << 16) | ((uint32_t)(uint8_t)(s)[2] << 8) | (uint32_t)(uint8_t)(s)[3])

#define IMG4_OK                 0
#define IMG4_ERR_FORMAT         (-1)    /* not an Image4 file this reads */
#define IMG4_ERR_CERTIFICATE    (-2)    /* a certificate it cannot use */
#define IMG4_ERR_CHAIN          (-3)    /* the chain does not lead to the root key */
#define IMG4_ERR_SIGNATURE      (-4)    /* the manifest is not signed by the chain */
#define IMG4_ERR_DEVICE         (-5)    /* the manifest is for another chip, board, device or nonce */
#define IMG4_ERR_EPOCH          (-6)    /* the manifest is older than this device allows */
#define IMG4_ERR_TYPE           (-7)    /* the payload is not of the type asked for, or the manifest has no entry for it */
#define IMG4_ERR_DIGEST         (-8)    /* the payload is not the one the manifest vouches for */

/* the key at the top of the chain, built into the BootROM */
struct img4_root {
    const uint8_t *public_key;
    const uint8_t *subject;
    size_t subject_size;
};

/* what a manifest has to match */
struct img4_device {
    uint32_t chip;
    uint32_t board;
    uint64_t ecid;
    uint8_t nonce_hash[IMG4_NONCE_HASH_SIZE];
    uint64_t epoch;
};

extern struct img4_device img4_this_device;

/* the payload, inside the file that was checked */
struct img4_image {
    uint32_t type;
    const uint8_t *description;
    size_t description_size;
    const uint8_t *data;
    size_t size;
};

int img4_verify(const uint8_t *file, size_t size, const char *type, const struct img4_root *root, const struct img4_device *device, struct img4_image *image);
const char *img4_error(int error);

#endif /* IMG4_H */
