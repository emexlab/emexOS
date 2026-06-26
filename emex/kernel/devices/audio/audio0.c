/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: audio0.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#include "audio0.h"

#include <kernel/module/module.h>
#include <kernel/communication/serial.h>
#include <drivers/drivers.h>
#include <kernel/sound/sound.h>
#include <drivers/sound/layer.h>
#include <kernel/kernel_processes/bootscreen/log.h>

static int audio0_init(void)
{
    log("[AUDIO0]", "init /dev/audio0\n", d);

    int ret = audiodrv_init();
    if (ret != 0) {
        log("[AUDIO0]", "no audio hardware found\n", warning);
        return -1;
    }

    log("[AUDIO0]", "audio layer ready\n", success);
    return 0;
}

static void audio0_fini(void) { /* nothing to tear down for now */ }

static void *audio0_open(const char *path)
{
    (void)path;

    if (!sound_available())
        return NULL;

    return (void *)1;
}

static int audio0_read(void *handle, void *buf, size_t count, u64 offset)
{
    (void)handle;
    (void)buf;
    (void)count;
    (void)offset;
    return 0;
}

static int audio0_write(void *handle, const void *buf, size_t count, u64 offset)
{
    (void)handle;
    (void)offset;

    if (!buf || !count)
        return -1;

    return audiodrv_write(buf, count);
}

driver_module audio0_module = {
    .name    = AUDIO0NAME,
    .mount   = AUDIO0PATH,
    .version = AUDIO0UNIVERSAL,
    .init    = audio0_init,
    .fini    = audio0_fini,
    .open    = audio0_open,
    .read    = audio0_read,
    .write   = audio0_write,
};
