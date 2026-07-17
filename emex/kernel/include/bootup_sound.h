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
