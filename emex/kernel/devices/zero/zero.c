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

#include "zero.h"
#include <kernel/module/module.h>
#include <kernel/graph/graphics.h>
#include <kernel/communication/serial.h>

#include <kernel/graph/lib/string.h>

#include <drivers/drivers.h>

static int zero_mod_init(void) {
    log("[ZERO]", "init /dev/zero\n", d);
    return 0;
}

static void zero_mod_fini(void) {}

static void *zero_open(const char *path) {
    (void)path;
    return (void *)1;
}

// write buffer full with 0s
static int zero_read(void *handle, void *buf, size_t count, u64 offset) {
    (void)handle;
    (void)offset;
    if (buf && count > 0) {
    	memset(buf, 0, count);
    }
    return (int)count;
}

static int zero_write(void *handle, const void *buf, size_t count, u64 offset) {
    (void)handle;
    (void)buf;
    (void)offset;
    return (int)count;
}

driver_module zero_module = {
    .name    = ZERNAME,
    .mount   = ZERPATH,
    .version = ZERUNIVERSAL,
    .init    = zero_mod_init,
    .fini    = zero_mod_fini,
    .open    = zero_open,
    .read    = zero_read,
    .write   = zero_write,
};
