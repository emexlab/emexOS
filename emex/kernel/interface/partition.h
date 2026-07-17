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

#ifndef PARTITION_H
#define PARTITION_H

#include <types.h>

#define MAX_PARTITIONS   4

#define PARTSECSIZE  65536

typedef struct
{
    u8     valid;
    u8     bootable;
    u8     type;

    u32     start_lba;
    u32     sector_count;

    char    type_name[32];
} partition_info_t;

int partition_init(void);
int partition_get_count(void);
partition_info_t* partition_get_info(int index);
int partition_format_disk_fat32(void);
int partition_needs_format(void);

#endif
