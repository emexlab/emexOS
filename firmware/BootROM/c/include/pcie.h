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

#ifndef FW_PCIE_H
#define FW_PCIE_H

#include <fw.h>

#define PCIE_RC_BASE            (EMEX64_MMIO_BASE + EMEX64_PAGE_SIZE * 8)
#define PCIE_RC_REG_ID          0x00
#define PCIE_RC_REG_ECAM_BASE   0x08
#define PCIE_RC_REG_ECAM_BUSES  0x10
#define PCIE_RC_REG_MMIO_BASE   0x18
#define PCIE_RC_REG_MMIO_SIZE   0x20
#define PCIE_RC_ID_MAGIC        0x49435045UL

#define PCIE_CFG_ID             0x00
#define PCIE_CFG_CONTROL        0x08
#define PCIE_CFG_STATUS         0x10
#define PCIE_CFG_IRQ            0x18
#define PCIE_CFG_HEADER         0x20
#define PCIE_CFG_BAR(n)         (0x40 + (n) * 0x10)
#define PCIE_CFG_BAR_SIZE(n)    (0x48 + (n) * 0x10)

#define PCIE_CONTROL_MEM        (1UL << 0)
#define PCIE_CONTROL_MASTER     (1UL << 1)
#define PCIE_CONTROL_IRQ        (1UL << 2)

#define PCIE_MAX_BARS           6
#define PCIE_MAX_FOUND          32

struct pcie_dev {
    u8 bus, dev, fn;
    u16 vendor, device;
    u32 class_code;             /* class << 16 | subclass << 8 | interface */
    u8 revision;
    u64 cfg;                    /* address of its configuration page */
    u64 bar[PCIE_MAX_BARS];
    u64 bar_size[PCIE_MAX_BARS];
    u64 irq;
};

/* enumerates bus 0, assigns BARs, returns the number of functions (< 0: no root complex) */
int pcie_init(void);
int pcie_count(void);
struct pcie_dev *pcie_get(int index);
struct pcie_dev *pcie_find_class(u32 class_code, int nth);
void pcie_enable(struct pcie_dev *d, u64 control);

static inline u64 pcie_bar_read(struct pcie_dev *d, int bar, u64 reg)
{
    return mmio_read64(d->bar[bar] + reg);
}

static inline void pcie_bar_write(struct pcie_dev *d, int bar, u64 reg, u64 value)
{
    mmio_write64(d->bar[bar] + reg, value);
}

#endif /* FW_PCIE_H */
