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

#include "fm.h"
#include <kernel/communication/serial.h>
#include <kernel/kernel_processes/tm/cfg/stdclrs.h>
#include <kernel/graph/theme.h>

#include <kernel/arch/x86_64/exceptions/panic.h>

static font_type_t current_font_type = FONT_8X8_BOLD; // default font
static const font_t *current_font = NULL;

/*
 * the font manager doesnt use the kernel process manager
 * but its still a 'process' in kinda way... like its used for graphics
 */

 /*
  * unicode_to_glyph_index made by dexoron 18.2.2026
  */
static u32 unicode_to_glyph_index(u32 cp)
{
    if (cp == 0x00A0) return (u32)' '; // NBSP

    // German umlauts/custom symbols in dedicated slots to avoid CP1251 conflicts.
    if (cp == 0x00E4) return 0x80; // ä
    if (cp == 0x00F6) return 0x81; // ö
    if (cp == 0x00FC) return 0x82; // ü
    if (cp == 0x00DF) return 0x83; // ß

    // Current glyph atlas is 256-entry, so only Latin-1 can map directly.
    if (cp <= 0x00FF) return cp;

    // Cyrillic mapped to CP1251 slots:
    // Ё=0xA8, ё=0xB8, А..Я=0xC0..0xDF, а..я=0xE0..0xFF (Russian Lang)
    if (cp == 0x0401) return 0xA8; // Ё
    if (cp == 0x0451) return 0xB8; // ё
    if (cp >= 0x0410 && cp <= 0x042F) return 0xC0 + (cp - 0x0410); // А..Я
    if (cp >= 0x0430 && cp <= 0x044F) return 0xE0 + (cp - 0x0430); // а..я

    return (u32)'?';
}

void fm_init(void) {
    BOOTUP_PRINTBS("[FM] ", GFX_GRAY_70);
    BOOTUP_PRINTBS("start font manager\n", white());

    // default font
    current_font_type = CURRENT_FONT;
    current_font = &font_registry[CURRENT_FONT];

    BOOTUP_PRINTBS("[FM] ", GFX_GRAY_70);
    BOOTUP_PRINTBS("default: ", white());
    BOOTUP_PRINTBS(current_font->name, GFX_ST_CYAN);
    BOOTUP_PRINTBS("\n", white());
}

int f_setcontext(font_type_t font_type)
{
    if (font_type >= FONT_COUNT) {
        BOOTUP_PRINT("[FM] ", GFX_GRAY_70);
        BOOTUP_PRINT("ERROR: invalid font type\n", red());
        return -1;
    }

    current_font_type = font_type;
    current_font = &font_registry[font_type];

    // do not print font switch messages during panic
    if (getcontext() == THEME_PANIC) return 0;

    //BOOTUP_PRINT("[FM] ", GFX_GRAY_70);
    //BOOTUP_PRINT("switched to: ", white());
    //BOOTUP_PRINT(current_font->name, GFX_ST_CYAN);
    //BOOTUP_PRINT("\n", white());

    return 0;
}

font_type_t fm_get_current_font(void) {
    return current_font_type;
}

const u8* fm_get_glyph_cp(u32 codepoint) {
    if (!current_font || !current_font->data) return NULL;
    if (current_font->char_height == 0) return NULL;
    if (current_font->row_bytes == 0 || current_font->row_bytes > 4) return NULL;

    u32 glyph_index = 0;
    if (current_font->unicode_direct) {
        glyph_index = codepoint;
        if (glyph_index >= current_font->glyph_count) {
            glyph_index = (u32)'?';
        }
    } else {
        glyph_index = unicode_to_glyph_index(codepoint);
    }

    if (glyph_index >= current_font->glyph_count) return NULL;

    const u8 *font_data = (const u8 *)current_font->data;
    u32 glyph_stride = current_font->char_height * current_font->row_bytes;
    return &font_data[glyph_index * glyph_stride];
}

u32 fm_get_glyph_row_bytes(void) {
    if (!current_font) return 1;
    if (current_font->row_bytes == 0) return 1;
    return current_font->row_bytes;
}

u32 fm_get_glyph_lsb_left(void) {
    if (!current_font) return 0;
    return current_font->lsb_left ? 1u : 0u;
}
u32 fm_get_char_width(void) {
    if (!current_font) return 8;
    return current_font->char_width;
}
u32 fm_get_char_height(void) {
    if (!current_font) return 8;
    return current_font->char_height;
}
const char* fm_get_font_name(void) {return current_font->name;}
