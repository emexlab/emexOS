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

#ifndef CMOS_H
#define CMOS_H

#include <types.h>

// time
typedef struct {
    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u8 year;     // 2-digit == 2025 ->25
} cmos_time_t;

void cmos_read_time(cmos_time_t *time);
u64 cmos_get_unix_timestamp(void);
void GetCMOSTime(void);
/*void outb(u16 port, u8 value);
char inb(u16 port);*/
USHORT GetCMOSMem(void);
void GetCMOSDate(void);

#endif
