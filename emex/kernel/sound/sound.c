/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: sound.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#include "sound.h"

int sound_available(void)
{
    return audiodrv_available();
}
