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

#include "kbd.h"

#include <kernel/module/module.h>
#include <drivers/ps2/keyboard/keyboard.h>
#include <kernel/communication/serial.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <kernel/graph/lib/string.h>

#include <drivers/drivers.h>

static int kbd_dev_init(void) {
    log("[KBD]", "init /dev/input/keyboard0\n", d);
    // keyboard_module is gone...
    keyboard_init();
    return 0;
}

static void kbd_dev_fini(void) {
    irq_unregister_handler(1);
}
static void *kbd_dev_open(const char *path) {
    (void)path;
    return (void *)1; // dummy handle
}

static int kbd_dev_read(void *handle, void *buf, size_t count, u64 offset) {
    (void)handle;
    (void)offset;

    size_t ev_size = sizeof(key_event_t);
    size_t written = 0;
    u8 *out = (u8 *)buf;

    // drain as many events as fit in buf
    while (written + ev_size <= count && keyboard_has_key()) {
        key_event_t ev;
        if (keyboard_get_event(&ev)) {
            u8 *src = (u8 *)&ev;
            for (size_t i = 0; i < ev_size; i++) {
                out[written + i] = src[i];
            }
            written += ev_size;
        }
    }

    return (int)written;
}

// there is no write for input devices
static int kbd_dev_write(void *handle, const void *buf, size_t count, u64 offset) {
    (void)handle;
    (void)buf;
    (void)count;
    (void)offset;
    return -1;
}

driver_module kbd_dev_module = {
    .name    = KBDNAME,
    .mount   = KBDPATH,
    .version = KBDUNIVERSAL,
    .init    = kbd_dev_init,
    .fini    = kbd_dev_fini,
    .open    = kbd_dev_open,
    .read    = kbd_dev_read,
    .write   = kbd_dev_write,
};
