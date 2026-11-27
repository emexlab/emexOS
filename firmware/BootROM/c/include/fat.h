/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.1
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.1
 * (EPSL 1.1).
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

#ifndef FAT_H
#define FAT_H

#include <stddef.h>
#include <stdint.h>

#define FAT_OK              0
#define FAT_ERR_IO          (-1)
#define FAT_ERR_NO_VOLUME   (-2)
#define FAT_ERR_NOT_FOUND   (-3)
#define FAT_ERR_NOT_FILE    (-4)
#define FAT_ERR_TOO_LARGE   (-5)
#define FAT_ERR_CORRUPT     (-6)

typedef int (*fat_read_fn)(void *context, uint64_t lba, uint32_t count, void *buffer);

struct fat_volume {
    fat_read_fn read;
    void *context;
    uint64_t start;
    uint32_t sector_size;
    uint32_t scale;
    uint32_t cluster_sectors;
    uint32_t cluster_size;
    uint32_t fat_start;
    uint32_t root_start;
    uint32_t root_sectors;
    uint32_t root_cluster;
    uint32_t data_start;
    uint32_t clusters;
    int type;
};

int fat_mount(struct fat_volume *volume, fat_read_fn read, void *context);
int fat_mount_at(struct fat_volume *volume, fat_read_fn read, void *context, uint64_t start);
int fat_size(struct fat_volume *volume, const char *path, uint64_t *size);
int fat_load(struct fat_volume *volume, const char *path, void *buffer, uint64_t room, uint64_t *size);

#endif /* FAT_H */