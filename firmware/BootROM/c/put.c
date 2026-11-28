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
#include <pcie.h>
#include <display.h>
#include <uart.h>

u64 gui_fb;
u16 gui_width;
u16 gui_height;
u64 display_bar;

static void (*putc_fn)(int c);

static void putc_uart(int c)
{
    uart_putc((char)c);
}

void fw_putc(int c)
{
    if(putc_fn == NULL)
    {
        putc_uart(c);
        return;
    }
    putc_fn(c);
}

void fw_puts(const char *s)
{
    while(*s)
    {
        fw_putc(*s++);
    }
}

int display_init(void)
{
    /* are we looking at PCIe Root Complex? >~< */
    if((mmio_read64(PCIE_RC_BASE + PCIE_RC_REG_ID) & 0xFFFFFFFF) != PCIE_RC_ID_MAGIC)
    {
        /* huh? where PCIe?? */
        return 0;
    }

    u64 ecam = mmio_read64(PCIE_RC_BASE + PCIE_RC_REG_ECAM_BASE);
    u64 cfg = 0;
    for(u64 dev = 0; dev < 32; dev++)
    {
        if((mmio_read64(ecam + (dev << 16) + PCIE_CFG_ID) & 0xFFFFFFFF) == GPU_PCI_ID)
        {
            cfg = ecam + (dev << 16);
            break;
        }
    }
    if(cfg == 0)
    {
        return 0;
    }

    u64 bar = mmio_read64(PCIE_RC_BASE + PCIE_RC_REG_MMIO_BASE) + mmio_read64(PCIE_RC_BASE + PCIE_RC_REG_MMIO_SIZE) - mmio_read64(cfg + PCIE_CFG_BAR_SIZE(0));
    mmio_write64(cfg + PCIE_CFG_BAR(0), bar);
    mmio_write64(cfg + PCIE_CFG_CONTROL, PCIE_CONTROL_MEM | PCIE_CONTROL_MASTER);
    display_bar = bar;

    /* a monitor on the DisplayPort connector? */
    if(!(mmio_read64(bar + GPU_DP_STATUS) & GPU_DP_HPD))
    {
        return 0;
    }
    mmio_write64(bar + GPU_DP_LINK, 1);
    mmio_write64(bar + GPU_DP_AUX_ADDR, GPU_AUX_EDID + 54);
    u64 dtd = mmio_read64(bar + GPU_DP_AUX_DATA);
    u64 width = ((dtd >> 16) & 0xFF) | ((dtd >> 36) & 0xF) << 8;
    u64 height = ((dtd >> 40) & 0xFF) | (dtd >> 60) << 8;
    if(width == 0 || height == 0)
    {
        width = 640;
        height = 480;
    }

    width &= ~7UL;

    u64 fb = (mmio_read64(EMEX64_MC_REG_SIZE) - width * height) & ~(EMEX64_PAGE_SIZE - 1);
    memset((void *)fb, 0, width * height);

    mmio_write64(bar + GPU_SCAN_BASE, fb);
    mmio_write64(bar + GPU_SCAN_WIDTH, width);
    mmio_write64(bar + GPU_SCAN_HEIGHT, height);
    mmio_write64(bar + GPU_SCAN_STRIDE, width);
    mmio_write64(bar + GPU_SCAN_FORMAT, GPU_FORMAT_I8);
    mmio_write64(bar + GPU_SCAN_CTRL, 1);

    gui_fb = fb;
    gui_width = (u16)width;
    gui_height = (u16)height;
    return 1;
}

void put_init(void)
{
    if(!display_init())
    {
        putc_fn = putc_uart;
        return;
    }
    putc_fn = gui_putc;

    fw_printf("BootROM %d.%d.%d\n", BOOTROM_VERSION_MAJOR, BOOTROM_VERSION_MINOR, BOOTROM_VERSION_PATCH);
}

void put_mem(u64 bytes,
             u64 unit_of)
{
    static const char *units[] = { "B", "KB", "MB", "GB", "TB" };

    u64 x = unit_of ? unit_of : bytes;
    int unit = 0;
    while(x >= 1024 && unit < 4)
    {
        x /= 1024;
        unit++;
    }

    u64 size = 1;
    for(int i = 0; i < unit; i++)
    {
        size *= 1024;
    }

    fw_printf("%lu", bytes / size);
    if(unit != 0)
    {
        fw_printf(".%lu", (bytes % size) * 10 / size);
    }
    fw_printf("%s ", units[unit]);
}
