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

#ifndef FW_BLK_H
#define FW_BLK_H

#include <fw.h>
#include <pcie.h>

#define BLK_MAX_DEVICES     16
#define BLK_SECTOR_SIZE     512

/* same values as E64BOOT_DEVICE_* in <EmexToolchain/Boot/e64boot.h> */
#define BLK_KIND_SATA       0
#define BLK_KIND_USB        1

struct blkdev {
    char name[8];               /* "sata0", "usb1", ... */
    int kind;                   /* BLK_KIND_* */
    struct pcie_dev *ctrl;      /* host controller */
    int port;                   /* SATA port / USB root port */
    u64 sectors;
    int readonly;

    /* return 0 or a driver error code for strerror() */
    int (*read)(struct blkdev *b, u64 lba, u64 count, void *buffer);
    int (*write)(struct blkdev *b, u64 lba, u64 count, const void *buffer);
    const char *(*strerror)(struct blkdev *b, int err);
};

void blk_register(struct blkdev *b);
int blk_count(void);
struct blkdev *blk_get(int index);

#endif /* FW_BLK_H */
