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

#ifndef FONTS_H
#define FONTS_H

#include <types.h>

typedef enum {
    //FONT_8X12_BOLD = 0,
    //FONT_8X12 = 1,
    FONT_8X8_BOLD = 0,
    FONT_8X8 = 1,
    //FONT_8X16 = 4,
    //FONT_8X16_BOLD = 5,
    //FONT_16X32 = 6,
    //GOHUFONT = 7,
    FONT_COUNT = 2//8
} font_type_t;
typedef struct {
    const char *name;
    const void *data;
    u32 char_width;
    u32 char_height;
    u32 row_bytes;
    u32 lsb_left;
    u32 glyph_count;
    u32 unicode_direct;
} font_t;

// font registry
extern const font_t font_registry[FONT_COUNT];

#endif
