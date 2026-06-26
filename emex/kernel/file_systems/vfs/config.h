/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: config.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
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