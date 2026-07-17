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

//#include "init.h"
#include <kernel/file_systems/vfs/config.h>
#include <kernel/file_systems/vfs/sysfs/sysfs.h>
#include "vfs.h"
#include <kernel/mem/klime/klime.h>
#include <kernel/include/reqs.h>
#include <kernel/module/module.h>
#include <kernel/arch/x86_64/exceptions/panic.h>

#include <kernel/graph/lib/string.h>
#include <kernel/graph/theme.h>

void fs_system_init(void *klime)
{
    // set klime for all fs types (l: except diskfs's)
    fs_klime = klime;
    //fs_set_klime(klime);
    //tmpfs_set_klime(klime);
    //devfs_set_klime(klime);

    // init core
    fs_init();

    // register fs types
    tmpfs_register();
    devfs_register();
    procfs_register();
    sysfs_register();

    log("[FS]", "mounting roots: \n", white());

    // mount file systems
    fs_mount(NULL, ROOT_MOUNT_DEFAULT, ROOTFS);

    fs_mkdir(BOOT_DIRECTORY);
    fs_mkdir(DEV_DIRECTORY);
    fs_mkdir(TMP_DIRECTORY);
    fs_mkdir(PROC_DIRECTORY);
    fs_mkdir(SYS_DIRECTORY);
    fs_mkdir(EMX_DIRECTORY);
    fs_mkdir(EMSYS_DIRECTORY);

    fs_mount(NULL, DEV_MOUNT_DEFAULT,  DEVFS);
    fs_mount(NULL, PROC_MOUNT_DEFAULT, PROCFS);
    fs_mount(NULL, SYS_MOUNT_DEFAULT,  SYSFS);
}

void fs_register_mods()
{
    int total = module_get_count();
    int dev_cnt = 0; // device count

    log("[FS]", "scann modules:\n", d);

    for (int i = 0; i < total; i++) {
        driver_module *mod = module_get_by_index(i);

        // only registers modules with /dev/* mount paths
        // these are device drivers that need devfs nodes
        if (mod && mod->mount && str_starts_with(mod->mount, _DEV)) {
            //printf("reg to: %s\n", mod->name);
            if (devfs_register_device(mod) == 0) {
                dev_cnt++;
            }
        }
    }

    log("[FS]", "registered ", d);
    BOOTUP_PRINT_INT(dev_cnt, white()); // device count
    BOOTUP_PRINT(" device(s)\n", white());
}

void fs_create_test_file(void) {
    // create and write test file in tmpfs
    // Write only
    int fd = fs_open("/tmp/t", O_CREAT | O_WRONLY);
    if (fd < 0) {
        log("[FS]", "failed to create file\n", error);
        return;
    }

    fs_write(fd, "this is a test for fs this file has no other use", str_len("this is a test for fs this file has no other use"));
    fs_close(fd);

   log("[FS]", "created ", d);
    BOOTUP_PRINT("/tmp/t", white());
    BOOTUP_PRINT("\n", white());
}
