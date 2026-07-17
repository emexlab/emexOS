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

#include "mbr.h"
#include <drivers/storage/ata/disk.h>
#include <kernel/mem/lib/main.h>
#include <kernel/communication/serial.h>

int mbr_read(u8 *buffer) {
    ATAdevice_t *device = ATAget_device(0);
    if (!device) {
        printf("[MBR] not found\n");
        return -1;
    }
    if (ATAread_sectors(device, 0, 1, (u16*)buffer) != 0) {
        printf("[MBR] Failed: read MBR sector\n");
        return -1;
    }

    return 0;
}

int mbr_write(const mbr_t *mbr) {
    ATAdevice_t *device = ATAget_device(0);
    if (!device) {
        printf("[MBR] not found\n");
        return -1;
    }
    if (ATAwrite_sectors(device, 0, 1, (u16*)mbr) != 0) {
        printf("[MBR] failed: write MBR sector\n");
        return -1;
    }

    return 0;
}

int mbr_is_valid(const mbr_t *mbr) {
    return mbr->signature == MBR_SIGNATURE;
}

void mbr_create_empty(mbr_t *mbr) {
    memset(mbr, 0, sizeof(mbr_t));
    mbr->signature = MBR_SIGNATURE;
}

int mbr_create_partition(mbr_t *mbr, u8 index, u8 type, u32 start_lba, u32 sector_count) {
    if (index >= 4) {
        return -1;
    }

    mbr_partition_t *part = &mbr->partitions[index];

    part->status = (index == 0) ? 0x80 : 0x00; // bootable
    part->partition_type = type;
    part->first_lba = start_lba;
    part->sector_count = sector_count;
    part->first_chs[0] = 0;
    part->first_chs[1] = 0;
    part->first_chs[2] = 0;
    part->last_chs[0] = 0xFF;
    part->last_chs[1] = 0xFF;
    part->last_chs[2] = 0xFF;

    return 0;
}

const char* mbr_get_partition_type_name(u8 type) {
    switch (type) {
        case MBR_PARTITION_EMPTY:     return "Empty";
        case MBR_PARTITION_FAT12:     return "FAT12";
        case MBR_PARTITION_FAT16:     return "FAT16";
        case MBR_PARTITION_EXTENDED:  return "Extended";
        case MBR_PARTITION_FAT16B:    return "FAT16B";
        case MBR_PARTITION_NTFS:      return "NTFS";
        case MBR_PARTITION_FAT32:     return "FAT32";
        case MBR_PARTITION_FAT32_LBA: return "FAT32 LBA";
        case MBR_PARTITION_FAT16_LBA: return "FAT16 LBA";
        case MBR_PARTITION_LINUX:     return "Linux";
        case MBR_PARTITION_LINUX_SWAP:return "Linux Swap";
        case MBR_PARTITION_EFI:       return "EFI";
        default:                      return "Unknown";
    }
}
