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

#ifndef NVRAM_H
#define NVRAM_H

#include <stddef.h>
#include <stdint.h>

#define NVRAM_HEADER_SIZE       16
#define NVRAM_SIG_COMMON        0x70
#define NVRAM_NAME_COMMON       "common"

uint8_t nvram_checksum(const uint8_t header[NVRAM_HEADER_SIZE]);
int nvram_common(const uint8_t *nvram, size_t size, size_t *offset, size_t *length);
int nvram_get(const uint8_t *vars, size_t size, const char *name, const char **value, size_t *value_size);

#endif /* NVRAM_H */
