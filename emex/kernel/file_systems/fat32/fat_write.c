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

#include "fat32_internal.h"
#include <kernel/mem/lib/main.h>
#include <kernel/communication/serial.h>

// fat32.c would get to big when
// everything is in that like syscalls.c...........
//
void flush_fat(void)
{
    u32 fat_start = partition_start_lba + bootSector.reserved_sectors;
    u32 fat2 = fat_start + bootSector.fat_size_32;

    for (u32 i = 0; i < bootSector.fat_size_32; i++) {
        ATAwrite_sectors(
        	disk_device, fat_start + i, 1, (u16*)((u8*)FAT + i * bootSector.bytes_per_sector)
        );
    }
    for (u32 i = 0; i < bootSector.fat_size_32; i++) {
        ATAwrite_sectors(
        	disk_device, fat2 + i, 1, (u16*)((u8*)FAT + i * bootSector.bytes_per_sector)
        );
    }
}

uint32_t alloc_cluster(void) {
    uint32_t total = (bootSector.fat_size_32 * bootSector.bytes_per_sector) / 4;

    for (uint32_t i = 2; i < total; i++) {
        if ((FAT[i] & 0x0FFFFFFF) == 0) {
            FAT[i] = 0x0FFFFFFF;
            return i;
        }
    }

    return 0;
}

int fat32_write(file_t *file, const void *buffer, size_t size)
{
	size_t written = 0;
 	uint32_t csize = bootSector.sectors_per_cluster * bootSector.bytes_per_sector;
    uint32_t first = file->entry->firstClusterLow | (file->entry->firstClusterHigh << 16);

    if (!fat32_initialized || !file || !buffer || size == 0) return -1;

    if (first == 0 || first == 1) {
        first = alloc_cluster();
        if (!first) return -1;

        file->entry->firstClusterLow = first & 0xFFFF;
        file->entry->firstClusterHigh = (first >> 16) & 0xFFFF;
    }

    uint32_t pos = file->readPos;

    while (written < size)
    {
        uint32_t idx = pos / csize;
        uint32_t off = pos % csize;
        uint32_t to_write = csize - off;
        uint32_t cur = first;

        for (uint32_t i = 0; i < idx; i++) {
            uint32_t next = FAT[cur] & 0x0FFFFFFF;
            if (next >= 0x0FFFFFF8) {
                uint32_t newc = alloc_cluster();
                if (!newc) goto done;
                FAT[cur] = newc;
                cur = newc;
            } else {
                cur = next;
            }
        }

        // read modify write  for partial cluster writes
        u32 lba = cluster_to_Lba(cur);
        static u8 tmp[4096] __attribute__((aligned(16)));

        for (
        	u32 s = 0; s < bootSector.sectors_per_cluster; s++
        ) {
        	ATAread_sectors(
         		disk_device, lba + s, 1,
           		(u16*) (tmp + s * bootSector.bytes_per_sector)
         	);
        }

        if (to_write > size - written) to_write = (uint32_t)(size - written);

        memcpy(tmp + off, (u8*)buffer + written, to_write);

        for (
        	u32 s = 0; s < bootSector.sectors_per_cluster; s++
        ){
            ATAwrite_sectors(
            	disk_device, lba + s, 1,(u16*)(tmp + s * bootSector.bytes_per_sector)
            );
        }

        written += to_write;
        pos += to_write;
    }

done:
    file->readPos = pos;

    if (
    	pos > file->entry->fileSize
    ) file->entry->fileSize = pos;

    flush_fat();
    return (int)written;
}
