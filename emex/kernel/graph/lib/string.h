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

#ifndef STRING_H
#define STRING_H

#include <types.h>
//#include "print.h"
//#include "log.h"
#include <kernel/kernel_processes/tm/cfg/stdclrs.h>
#include <kernel/kernel_processes/tm/cfg/doccr.h>

#include <kernel/kernel_processes/bootscreen/print.h>
#include <kernel/kernel_processes/bootscreen/log.h>

void str_copy(char *dest, const char *src);
void str_append(char *dest, const char *src);
void str_append_uint(char *dest, u32 num);
int str_len(const char *str);
void print_str(const char *str, u32 color);
int str_starts_with(const char *str, const char *prefix);
int str_equals(const char *s1, const char *s2);
int str_contains(const char *str, const char *substr);
void str_to_upper(char *str);

void str_append_char(char *dest, char c);


void str_from_int(char *buf, int value);
void str_from_hex(char *buf, u64 value);
void IntToString(int value, char *buffer); //print.c

#endif
