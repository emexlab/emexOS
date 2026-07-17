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

#ifndef TIMER_H
#define TIMER_H

#include <types.h>
#include <kernel/multitasking/multitasking.h>

#define TIMER_FREQUENCY 1000  // 1000 Hz = 1ms Ticks
#define MAX_TIMER_CALLBACKS 8

//callback type
typedef void (*timer_callback_t)(void);

void timer_init(u32 frequency);
void timer_wait(u32 ticks);
u64 timer_get_ticks(void);
u64 timer_get_seconds(void);
u64 timer_get_milliseconds(void);
int timer_register_callback(timer_callback_t callback);
void timer_unregister_callback(timer_callback_t callback);

// for  uptime command
void timer_set_boot_time(void);
u64 timer_get_uptime_seconds(void);
void timer_print_uptime(void);

#endif
