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

#ifndef DRIVERS_PS2_KEYBOARD_KEYBOARD_H
#define DRIVERS_PS2_KEYBOARD_KEYBOARD_H

#include <types.h>
#include <kernel/module/module.h>

#define KEY_BUFFER_SIZE 128

#define KEY_CTRL_MASK 0x80000000
#define KEY_SHIFT_MASK 0x40000000
#define KEY_ALT_MASK 0x20000000

// non-ASCII keyss
#define KEY_ARROW_UP    0x100
#define KEY_ARROW_DOWN  0x101
#define KEY_ARROW_LEFT  0x102
#define KEY_ARROW_RIGHT 0x103
#define KEY_HOME        0x104
#define KEY_END         0x105
#define KEY_PAGE_UP     0x106
#define KEY_PAGE_DOWN   0x107
#define KEY_DELETE      0x108
#define KEY_INSERT      0x109
#define KEY_F1          0x110
#define KEY_F2          0x111
#define KEY_F3          0x112
#define KEY_F4          0x113
#define KEY_F5          0x114
#define KEY_F6          0x115
#define KEY_F7          0x116
#define KEY_F8          0x117
#define KEY_F9          0x118
#define KEY_F10         0x119
#define KEY_F11         0x11A
#define KEY_F12         0x11B

// modifier press/release events
#define KEY_LSHIFT      0x180
#define KEY_RSHIFT      0x181
#define KEY_LCTRL       0x182
#define KEY_LALT        0x183

// key event structure
typedef struct {
    u32 keycode;  // ASCII char or special code
    u32 modifiers; // CTRL, SHIFT, ALT flags
    u8 pressed;    // 1 = pressed, 0 = released
} key_event_t;

typedef struct {
    key_event_t buffer[KEY_BUFFER_SIZE];
    int read_pos;
    int write_pos;
    int count;
} key_buffer_t;

void keyboard_init(void);
void keyboard_poll(void);
int keyboard_has_key(void);
int keyboard_get_event(key_event_t *event);
char keyboard_get_key(void);

extern driver_module keyboard_module;

#endif /* DRIVERS_PS2_KEYBOARD_KEYBOARD_H */
