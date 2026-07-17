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

#include "mouse0.h"
#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/lib/string.h>

#include <drivers/drivers.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <drivers/ps2/mouse/mouse.h>

static int mouse0_init(void) {
    log("[MOUSE]", "init /dev/input/mouse0\n", d);
    mouse_init();
    return 0;
}
static void mouse0_fini(void) {
    irq_unregister_handler(12);
}
static void *mouse0_open(const char *path) {
    (void)path;
    return (void *)1;
}

static int mouse0_read(void *handle, void *buf, size_t count, u64 offset) {
    (void)handle;
    (void)offset;

    size_t ev_size = sizeof(mouse_event_t);
    size_t written = 0;
    u8 *out = (u8 *)buf;

    while (written + ev_size <= count && mouse_has_event()) {
        mouse_event_t ev;
        if (mouse_get_event(&ev)) {
            u8 *src = (u8 *)&ev;
            for (size_t i = 0; i < ev_size; i++)
                out[written + i] = src[i];
            written += ev_size;
        }
    }
    return (int)written;
}

static int mouse0_write(void *handle, const void *buf, size_t count, u64 offset) {
    (void)handle; (void)buf; (void)count; (void)offset;
    return -1;
}

driver_module mouse0_module = {
    .name    = MS0NAME,
    .mount   = MS0PATH,
    .version = MS0UNIVERSAL,
    .init    = mouse0_init,
    .fini    = mouse0_fini,
    .open    = mouse0_open,
    .read    = mouse0_read,
    .write   = mouse0_write,
};
