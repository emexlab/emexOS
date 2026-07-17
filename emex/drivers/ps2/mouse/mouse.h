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
#include <types.h>

#define MOUSE_BUF_SIZE 64

#define PS2_DATA 0x60
#define PS2_CMD  0x64

typedef struct {
    int dx, dy; // relative movements
    int abs_x, abs_y;
    unsigned char buttons;
} mouse_event_t;

#define MOUSE_BTN_LEFT   (1<<0)
#define MOUSE_BTN_RIGHT  (1<<1)
#define MOUSE_BTN_MIDDLE (1<<2)

void mouse_init(void);
int mouse_has_event(void);
int mouse_get_event(mouse_event_t *ev);
