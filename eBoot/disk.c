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

#include "disk.h"
#include <EmexToolchain/Boot/e64boot.h>

#define MMIO_BASE               0x0020000000000000UL
#define PCIE_RC_BASE            (MMIO_BASE + 0x2000UL * 8)
#define PCIE_RC_REG_ECAM_BASE   0x08
#define PCIE_CFG_ID             0x00
#define PCIE_CFG_CONTROL        0x08
#define PCIE_CFG_BAR0           0x40
#define PCIE_CONTROL_MEM        (1UL << 0)
#define PCIE_CONTROL_MASTER     (1UL << 1)

#define SATA_PCI_CLASS          0x0106E6
#define SATA_PORT_BASE(p)       (0x100 + (p) * 0x100)
#define SATA_PxSSTS             0x00
#define SATA_PxCLB              0x18
#define SATA_PxCMD              0x20
#define SATA_PxCI               0x28
#define SATA_PxIS               0x30
#define SATA_SSTS_PRESENT       (1UL << 0)
#define SATA_CMD_START          (1UL << 0)
#define SATA_OP_READ_DMA_EXT    0x25
#define SATA_STATUS_DONE        (1UL << 0)
#define SATA_STATUS_ERROR       (1UL << 1)

#define SECTORS_PER_COMMAND     128
#define POLL_LIMIT              100000000UL

struct slot {
    uint64_t opcode;
    uint64_t lba;
    uint64_t count;
    uint64_t buffer;
    volatile uint64_t status;
    volatile uint64_t bytes;
    uint64_t reserved[2];
};

static struct slot slots[32] __attribute__((aligned(64)));

static uint64_t mmio_read(uint64_t addr)
{
    return *(volatile uint64_t *)addr;
}

static void mmio_write(uint64_t addr, uint64_t value)
{
    *(volatile uint64_t *)addr = value;
}

static uint64_t port_reg(const struct disk *d, uint64_t reg)
{
    return d->bar + SATA_PORT_BASE(d->port) + reg;
}

int disk_open(struct disk *disk,
              uint64_t boot_device,
              uint64_t boot_port)
{
    if(E64BOOT_DEVICE_KIND(boot_device) != E64BOOT_DEVICE_SATA)
    {
        return DISK_ERR_KIND;
    }

    uint64_t dev = E64BOOT_DEVICE_DEV(boot_device);
    uint64_t fn = E64BOOT_DEVICE_FN(boot_device);
    if(E64BOOT_DEVICE_BUS(boot_device) != 0 || dev > 31 || fn > 7 || boot_port > 63)
    {
        return DISK_ERR_DEVICE;
    }
    uint64_t cfg = mmio_read(PCIE_RC_BASE + PCIE_RC_REG_ECAM_BASE) + (dev << 16) + (fn << 13);
    uint64_t id = mmio_read(cfg + PCIE_CFG_ID);
    uint64_t control = mmio_read(cfg + PCIE_CFG_CONTROL);
    if(((id >> 32) & 0xFFFFFF) != SATA_PCI_CLASS ||
       (control & (PCIE_CONTROL_MEM | PCIE_CONTROL_MASTER)) != (PCIE_CONTROL_MEM | PCIE_CONTROL_MASTER))
    {
        return DISK_ERR_DEVICE;
    }

    disk->bar = mmio_read(cfg + PCIE_CFG_BAR0);
    disk->port = boot_port;
    if(disk->bar == 0 || !(mmio_read(port_reg(disk, SATA_PxSSTS)) & SATA_SSTS_PRESENT))
    {
        return DISK_ERR_DEVICE;
    }

    mmio_write(port_reg(disk, SATA_PxCLB), (uint64_t)slots);
    mmio_write(port_reg(disk, SATA_PxCMD), SATA_CMD_START);
    return DISK_OK;
}

static int issue(const struct disk *d,
                 uint64_t lba,
                 uint64_t count,
                 uint64_t buffer)
{
    struct slot *s = &slots[0];
    uint64_t n;

    s->opcode = SATA_OP_READ_DMA_EXT;
    s->lba = lba;
    s->count = count;
    s->buffer = buffer;
    s->status = 0;
    s->bytes = 0;

    mmio_write(port_reg(d, SATA_PxIS), ~0UL);
    mmio_write(port_reg(d, SATA_PxCI), 1);
    for(n = 0; n < POLL_LIMIT; n++)
    {
        if(!(mmio_read(port_reg(d, SATA_PxCI)) & 1))
        {
            break;
        }
    }
    if(n == POLL_LIMIT || !(s->status & SATA_STATUS_DONE) || (s->status & SATA_STATUS_ERROR))
    {
        return DISK_ERR_IO;
    }

    return DISK_OK;
}

int disk_read(void *disk,
              uint64_t lba,
              uint32_t count,
              void *buffer)
{
    const struct disk *d = disk;
    uint64_t at = (uint64_t)buffer;

    while(count > 0)
    {
        uint32_t n = count < SECTORS_PER_COMMAND ? count : SECTORS_PER_COMMAND;
        if(issue(d, lba, n, at) != DISK_OK)
        {
            return DISK_ERR_IO;
        }
        lba += n;
        count -= n;
        at += (uint64_t)n * DISK_SECTOR_SIZE;
    }

    return DISK_OK;
}
