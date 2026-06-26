/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: random.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
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