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
