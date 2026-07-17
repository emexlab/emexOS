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

#include "random.h"
#include "urandom.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>

#include <drivers/drivers.h>

//
// for now random is just a alias from urandom
// cuz a real random would be way harder
//

static int random_init(void) {
    log("[RAND]", "init /dev/random (alias of urandom)\n", d);
    return 0;
}

static void random_fini(void) {}

driver_module random_module = {
    .name    = RNDNAME,
    .mount   = RNDPATH,
    .version = RNDUNIVERSAL,
    .init    = random_init,
    .fini    = random_fini,
    .open    = urandom_open_fn,
    .read    = urandom_read_fn,
    .write   = urandom_write_fn,
};
