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

#include "null.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>

#include <drivers/drivers.h>

static int null_mod_init(void) {
    log("[NULL]", "init /dev/null\n", d);
    return 0;
}

static void null_mod_fini(void) {}

static void *null_open(const char *path) {
    (void)path;
    return (void *)1;
}

static int null_read(void *handle, void *buf, size_t count, u64 offset) {
    (void)handle; (void)buf; (void)count; (void)offset;
    // eof
    return 0;
}
static int null_write(void *handle, const void *buf, size_t count, u64 offset) {
    (void)handle; (void)buf; (void)offset;
    return (int)count;
}

driver_module null_module = {
    .name    = NULNAME,
    .mount   = NULPATH,
    .version = NULUNIVERSAL,
    .init    = null_mod_init,
    .fini    = null_mod_fini,
    .open    = null_open,
    .read    = null_read,
    .write   = null_write,
};
