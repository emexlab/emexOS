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

#ifndef KEYBOARD_MAPS_H
#define KEYBOARD_MAPS_H

#include <types.h>

typedef struct {
    u8 normal[128];  // normal key mapping
    u8 shift[128];   // shift key mapping
} keymap_t;


int keymap_init(void);
const keymap_t* keymap_get_current(void);
const char* keymap_get_current_name(void);
int keymap_set(const char *name);

#endif
