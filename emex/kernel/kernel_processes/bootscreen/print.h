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

#ifndef PRINT_H
#define PRINT_H

#include <types.h>

#include "console/console.h"
#define SCREENMODE BS1

void putchar_bootstrap(char c, u32 color);
void printbs(const char *str, u32 color);

// text output functions
void putchar(char c, u32 color);
void string(const char *str, u32 color);
void print(const char *str, u32 color);
void printInt(int value, u32 color);
void printInt_to(int screen, int value, u32 color);
void print_to(int screen, const char *str, u32 color);
//void cprintf(const char *str, u32 color);

void reset_cursor(void);

#endif
