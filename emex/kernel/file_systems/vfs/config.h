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

#pragma once
extern char *logpath;
extern int init_boot_log;

void initvfs(void);

#define DISK_NAME "hdd0"

#define ROOTFS TMPFS // for now, soon its fat32/ext2
#define ROOT_MOUNT_DEFAULT "/"

#define HOME "/home"

#define FAT32 "fat32"
#define FAT32_DIRECTORY "/"

#define EXT2 "ext2"
#define EXT2_DIRECTORY "/"

#define TMPFS "tmpfs"
#define TMP_DIRECTORY "/tmp"

#define SYSFS "sysfs"
#define SYS_MOUNT_DEFAULT "/sys"
#define SYS_DIRECTORY "/sys"

#define DEVFS "devfs"
#define DEV_MOUNT_DEFAULT "/dev"
#define DEV_DIRECTORY "/dev"
#define _DEV "/dev/"

#define EMX_DIRECTORY "/emr" // emex system resources
#define EMSYS_DIRECTORY /*" /emr "*/EMX_DIRECTORY "/system"

#define BOOT_DIRECTORY "/boot"

#define PROCFS "procfs"
#define PROC_MOUNT_DEFAULT "/proc"
#define PROC_DIRECTORY "/proc"
