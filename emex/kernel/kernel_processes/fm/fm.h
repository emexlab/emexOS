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

#ifndef FM_H
#define FM_H

#include <types.h>
#include <kernel/graph/ui/fonts.h>

#define CURRENT_FONT FONT_8X8

void fm_init(void);
int f_setcontext(font_type_t font_type);
font_type_t fm_get_current_font(void);
const u8* fm_get_glyph_cp(u32 codepoint);
u32 fm_get_glyph_row_bytes(void);
u32 fm_get_glyph_lsb_left(void);

// get fm properties
u32 fm_get_char_width(void);
u32 fm_get_char_height(void);
const char* fm_get_font_name(void);

#endif
