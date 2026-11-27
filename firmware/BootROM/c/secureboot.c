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

#include <fw.h>
#include <secureboot.h>
#include <bootrom_key.h>
#include <monocypher-ed25519.h>

int secureboot_check(const void *file,
                     uint64_t size,
                     uint64_t *image_size)
{
    const u8 *bytes = file;

    if(size < SECUREBOOT_FOOTER_SIZE)
    {
        return SECUREBOOT_ERR_UNSIGNED;
    }

    uint64_t image = size - SECUREBOOT_FOOTER_SIZE;
    const u8 *signature = bytes + image;
    if(fw_memcmp(signature + SECUREBOOT_SIGNATURE_SIZE, SECUREBOOT_MAGIC, SECUREBOOT_MAGIC_SIZE) != 0)
    {
        return SECUREBOOT_ERR_UNSIGNED;
    }

    if(crypto_ed25519_check(signature, bootrom_public_key, bytes, image) != 0)
    {
        return SECUREBOOT_ERR_BAD_SIGNATURE;
    }

    *image_size = image;
    return SECUREBOOT_OK;
}
