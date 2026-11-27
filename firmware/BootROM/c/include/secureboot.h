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

#ifndef SECUREBOOT_H
#define SECUREBOOT_H

#include <stdint.h>

#define SECUREBOOT_MAGIC                "EMEXSIG1"
#define SECUREBOOT_MAGIC_SIZE           8
#define SECUREBOOT_SIGNATURE_SIZE       64
#define SECUREBOOT_FOOTER_SIZE          (SECUREBOOT_SIGNATURE_SIZE + SECUREBOOT_MAGIC_SIZE)

#define SECUREBOOT_OK                   0
#define SECUREBOOT_ERR_UNSIGNED         (-1)
#define SECUREBOOT_ERR_BAD_SIGNATURE    (-2)

int secureboot_check(const void *file, uint64_t size, uint64_t *image_size);

#endif /* SECUREBOOT_H */
