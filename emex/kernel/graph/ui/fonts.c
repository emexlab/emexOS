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

#include "fonts.h"
#include "fonts/font_8x8.h"
#include "fonts/font_8x8_bold.h"
//#include "fonts/font_8x16.h"
//#include "fonts/font_8x12_bold.h"
//#include "fonts/font_8x12.h"
//#include "fonts/font_8x16_bold.h"
//#include "fonts/font_16x32.h"
//#include "fonts/gohufont.h"

// font registery
const font_t font_registry[FONT_COUNT] = {
    /*[FONT_8X12_BOLD] = {
        .name = "8x12_BOLD",
        .data = &font_8x12_bold,
        .char_width = 8,
        .char_height = 12,
        .row_bytes = 2,
        .lsb_left = 1,
        .glyph_count = 2048,
        .unicode_direct = 1
    },
    [FONT_8X12] = {
        .name = "8x12",
        .data = &font_8x12,
        .char_width = 8,
        .char_height = 12,
        .row_bytes = 2,
        .lsb_left = 1,
        .glyph_count = 2048,
        .unicode_direct = 1
    },*/
    [FONT_8X8_BOLD] = {
        .name = "8x8_BOLD",
        .data = &font_8x8_bold,
        .char_width = 8,
        .char_height = 8,
        .row_bytes = 1,
        .lsb_left = 0,
        .glyph_count = 256,
        .unicode_direct = 0
    },
    /*[FONT_8X16] = {
        .name = "8x16",
        .data = &font_8x16,
        .char_width = 8,
        .char_height = 16,
        .row_bytes = 1,
        .lsb_left = 0,
        .glyph_count = 256,
        .unicode_direct = 0
    },
    [FONT_8X16_BOLD] = {
        .name = "8x16_BOLD",
        .data = &font_8x16_bold,
        .char_width = 8,
        .char_height = 16,
        .row_bytes = 1,
        .lsb_left = 0,
        .glyph_count = 256,
        .unicode_direct = 0
    },*/
    [FONT_8X8] = {
        .name = "8x8",
        .data = &font_8x8,
        .char_width = 8,
        .char_height = 8,
        .row_bytes = 1,
        .lsb_left = 0,
        .glyph_count = 2590,
        .unicode_direct = 0
    },
    /*[FONT_16X32] = {
        .name = "16x32",
        .data = &font_16x32,
        .char_width = 16,
        .char_height = 32,
        .row_bytes = 1,
        .lsb_left = 0,
        .glyph_count = 256,
        .unicode_direct = 0
    }*//*,
    [GOHUFONT] = {
        .name = "gohufont",
        .data = &gohufont_8x16,
        .char_width = 8,
        .char_height = 12
    }*/
};
