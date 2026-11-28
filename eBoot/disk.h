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

#ifndef EBOOT_DISK_H
#define EBOOT_DISK_H

#include <stdint.h>
#include "export.h"

#define DISK_SECTOR_SIZE    512

#define DISK_OK             0
#define DISK_ERR_KIND       (-1)
#define DISK_ERR_DEVICE     (-2)
#define DISK_ERR_IO         (-3)

struct disk {
    uint64_t bar;
    uint64_t port;
};

EBOOT_EXPORT int disk_open(struct disk *disk, uint64_t boot_device, uint64_t boot_port);
EBOOT_EXPORT int disk_read(void *disk, uint64_t lba, uint32_t count, void *buffer);

#endif /* EBOOT_DISK_H */
