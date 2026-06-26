/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: layer.c
 * CREATED BY: offihito
 * MODIFIED BY: --
 *
 */

#include "layer.h"
#include "ac97/ac97.h"


static audio_driver_t ac97_drv = {
    .init       = ac97_init,
    .write      = ac97_write,
    .set_rate   = ac97_set_rate,
    .set_volume = ac97_set_volume,
    .present    = ac97_present,
    .beep       = ac97_beep,
};

static audio_driver_t *active = NULL;

int audiodrv_init(void)
{
    if (ac97_drv.init() == 0) {
        active = &ac97_drv;
        return 0;
    }

    return -1;
}

int audiodrv_write(const void *buf, size_t len)
{
    if (!active)
        return -1;

    return active->write(buf, len);
}

int audiodrv_set_rate(u32 hz)
{
    if (!active)
        return -1;

    return active->set_rate(hz);
}

void audiodrv_set_volume(u8 master_atten, u8 pcm_atten)
{
    if (!active)
        return;

    active->set_volume(master_atten, pcm_atten);
}

int audiodrv_available(void)
{
    return active != NULL;
}

int audiodrv_beep(u32 freq_hz, u32 duration_ms)
{
    if (!active || !active->beep)
        return -1;

    return active->beep(freq_hz, duration_ms);
}
