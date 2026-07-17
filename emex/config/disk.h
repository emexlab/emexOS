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

#ifndef CONFIG_DISK_H
#define CONFIG_DISK_H

#include "system.h"

#define PARTITION_START_LBA 2048
#define PARTITION_TYPE_FAT32 0x0C  // FAT32 LBA

#define FAT32_CLUSTER_SIZE 8       // 4KB clusters
#define FAT32_RESERVED_SECTORS 32
#define FAT32_NUM_FATS 2
#define FAT32_ROOT_CLUSTER 2

#define FAT32_VOLUME_LABEL "EMEXOS"
#define FAT32_OEM_NAME "EMEX" KERNEL_DEFRELEASE

#endif /* CONFIG_DISK_H */
