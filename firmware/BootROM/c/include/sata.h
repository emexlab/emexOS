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

#ifndef FW_SATA_H
#define FW_SATA_H

#include <fw.h>
#include <pcie.h>
#include <blk.h>

#define SATA_PCI_CLASS              0x0106E6

#define SATA_REG_CAP                0x00
#define SATA_REG_GHC                0x08
#define SATA_REG_IS                 0x10
#define SATA_REG_PI                 0x18
#define SATA_GHC_RESET              (1UL << 0)

#define SATA_PORT_BASE(p)           (0x100 + (p) * 0x100)
#define SATA_PxSSTS                 0x00
#define SATA_PxSECTORS              0x08
#define SATA_PxSECSZ                0x10
#define SATA_PxCLB                  0x18
#define SATA_PxCMD                  0x20
#define SATA_PxCI                   0x28
#define SATA_PxIS                   0x30
#define SATA_PxIE                   0x38
#define SATA_PxSERR                 0x40

#define SATA_SSTS_PRESENT           (1UL << 0)
#define SATA_SSTS_READONLY          (1UL << 1)
#define SATA_CMD_START              (1UL << 0)

#define SATA_OP_READ_DMA_EXT        0x25
#define SATA_OP_WRITE_DMA_EXT       0x35
#define SATA_OP_FLUSH_CACHE_EXT     0xEA
#define SATA_OP_IDENTIFY            0xEC

#define SATA_STATUS_DONE            (1UL << 0)
#define SATA_STATUS_ERROR           (1UL << 1)

#define SATA_SECTOR_SIZE            512
#define SATA_MAX_DISKS              8

/* one command slot in memory (64 bytes) */
struct sata_slot {
    u64 opcode;
    u64 lba;
    u64 count;
    u64 buffer;
    volatile u64 status;
    volatile u64 bytes;
    u64 reserved[2];
};

struct sata_disk {
    struct blkdev blk;          /* registered as sata<index> */
    struct pcie_dev *ctrl;
    int index;                  /* sata<index> */
    int port;
    u64 sectors;
    int readonly;
    u64 max_sectors;            /* per command */
    char model[41];
    char serial[21];
    struct sata_slot *slots;    /* 32 slots, 64 byte aligned */
};

int sata_init(void);
int sata_disk_count(void);
struct sata_disk *sata_disk(int index);

/* returns 0 or the controller error code (> 0), -1 on timeout */
int sata_read(struct sata_disk *disk, u64 lba, u64 count, void *buffer);
int sata_write(struct sata_disk *disk, u64 lba, u64 count, const void *buffer);
const char *sata_strerror(int err);

#endif /* FW_SATA_H */
