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

#ifndef SOUND_LAYER_H
#define SOUND_LAYER_H

#include <types.h>


typedef struct {
    int  (*init)(void);
    int  (*write)(const void *buf, size_t len);
    int  (*set_rate)(u32 hz);
    void (*set_volume)(u8 master_atten, u8 pcm_atten);
    int  (*present)(void);
    int  (*beep)(u32 freq_hz, u32 duration_ms);
} audio_driver_t;

int  audiodrv_init(void);

int  audiodrv_write(const void *buf, size_t len);

int  audiodrv_set_rate(u32 hz);

void audiodrv_set_volume(u8 master_atten, u8 pcm_atten);

int  audiodrv_available(void);

int  audiodrv_beep(u32 freq_hz, u32 duration_ms);

#endif /* SOUND_LAYER_H */
