/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * emexOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * emexOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with emexOS. If not, see <https://www.gnu.org/licenses/>.
 */

#include "hdd0.h"
#include <kernel/graph/lib/string.h>

#include <drivers/drivers.h>
#include <kernel/mem/lib/main.h>
#include <types.h>

#define HDA_SECTOR_SIZE 512

/*
 * TODO
 * make disk layer and disconnect hda0 from ata
 */

static int ATAmodule_init(void)
{
    log("[HDA]", "init /dev/hda0\n", d);
    return 0;
}

static void ATAmodule_fini(void) {
    // cleanup if needed
}

static void *hda_open(const char *path) {
    (void)path;
    return ATAget_device(0);
}

static int hda_read(void *handle, void *buf, size_t count, u64 offset)
{
    ATAdevice_t *dev = (ATAdevice_t *)handle;
    if (!dev || !buf || count == 0) return -1;

    u64 lba = offset / HDA_SECTOR_SIZE;
    u32 lba_offset = offset % HDA_SECTOR_SIZE;
    u32 sectors = (u32)((lba_offset + count + HDA_SECTOR_SIZE - 1) / HDA_SECTOR_SIZE);

    // temp buffer for unaligned reads
    static u8 tmp[HDA_SECTOR_SIZE] __attribute__((aligned(16)));
    size_t remaining = count;
    u8 *dst = (u8 *)buf;

    for (u32 i = 0; i < sectors && remaining > 0; i++)
    {
        if (ATAread_sectors(dev, lba + i, 1, (u16 *)tmp) != 0) return -1;

        u32 src_off = (i == 0) ? lba_offset : 0;
        u32 to_copy = HDA_SECTOR_SIZE - src_off;
        if (to_copy > remaining) to_copy = (u32)remaining;

        memcpy(dst, tmp + src_off, to_copy);

        dst += to_copy;
        remaining -= to_copy;
    }
    return (int)(count - remaining);
}

static int hda_write(void *handle, const void *buf, size_t count, u64 offset)
{
    ATAdevice_t *dev = (ATAdevice_t *)handle;
    if (!dev || !buf || count == 0) return -1;

    u64 lba = offset / HDA_SECTOR_SIZE;
    u32 lba_offset = offset % HDA_SECTOR_SIZE;
    u32 sectors = (u32)((lba_offset + count + HDA_SECTOR_SIZE - 1) / HDA_SECTOR_SIZE);

    static u8 tmp[HDA_SECTOR_SIZE] __attribute__((aligned(16)));
    size_t remaining = count;
    const u8 *src = (const u8 *)buf;

    for (u32 i = 0; i < sectors && remaining > 0; i++)
    {
        u32 dst_off = (i == 0) ? lba_offset : 0;
        u32 to_copy = HDA_SECTOR_SIZE - dst_off;
        if (to_copy > remaining) to_copy = (u32)remaining;
        if (dst_off != 0 || to_copy != HDA_SECTOR_SIZE) {
            if (ATAread_sectors(dev, lba + i, 1, (u16 *)tmp) != 0) return -1;
        }

        memcpy(tmp + dst_off, src, to_copy);

        if (ATAwrite_sectors(dev, lba + i, 1, (u16 *)tmp) != 0) return -1;

        src += to_copy;
        remaining -= to_copy;
    }
    return (int)(count - remaining);
}

driver_module ata_module = {
    .name    = HDANAME,
    .mount   = HDAPATH,
    .version = HDAUNIVERSAL,
    .init    = ATAmodule_init,
    .fini    = ATAmodule_fini,
    .open    = hda_open,
    .read    = hda_read,
    .write   = hda_write,
};
