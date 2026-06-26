/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: bootup_sound.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once
#include <kernel/sound/sound.h>

static inline void bootup_sound()
{
	audiodrv_beep(1000, 140);
    audiodrv_beep(500, 240);
    audiodrv_beep(730, 110);
    audiodrv_beep(770, 25);
    //audiodrv_beep(780, 25);
    audiodrv_beep(790, 25);
    audiodrv_beep(800, 25);
    audiodrv_beep(850, 150);
    audiodrv_beep(990, 260);
    audiodrv_beep(1000, 250);
}