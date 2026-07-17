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
#include "stdclrs.h"
#include <kernel/graph/ui/fonts.h>

#include <kernel/file_systems/vfs/vfs.h> // fs_write & fs_open
#include <kernel/communication/serial.h>
#include <kernel/graph/lib/string.h>
#include <config/system.h>
#include <config/bootlogs.h>

//#define PCF_FONT_PATH "/boot/fonts/gohu-uni-14.pcf"

// ******************
// kernel theme:
//

//bootup
extern int init_boot_log;
#define BOOTUP_FONT FONT_8X8_DOS

//screen:
#define BOOTSCREEN_BG_COLOR GFX_ST_BLACK
#define BOOTSCREEN_COLOR GFX_ST_WHITE
#define LOGO_SCALE 2
#define BOOTUP_COLOR_THEME STD

//console
#define CONSOLE_FONT FONT_8X8_DOS

#define CONSOLESCREEN_BG_TYPE 0  // 0 = color, 1 = bmp picture
#define CONSOLESCREEN_BG_COLOR GFX_BG
#define CONSOLESCREEN_BG_IMAGE "/emr/assets/bg.bmp"

#define CONSOLESCREEN_COLOR GFX_WHITE
#define CONSOLE_COLOR_THEME FLU

//panic
#define PANIC_FONT FONT_8X8_BOLD
#define PANICSCREEN_BG_COLOR GFX_ST_RED
#define PANICSCREEN_FG_COLOR GFX_ST_RED + GFX_ST_WHITE
#define PANICSCREEN_COLOR GFX_ST_WHITE
#define PANICSCREEN_COLOR_R GFX_ST_RED
#define PANIC_COLOR_THEME STD
