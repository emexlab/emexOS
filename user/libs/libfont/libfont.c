/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: libfont.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "libfont.h"
#include "data/font8x12.h"
#include "data/font8x12_bold.h"

uint16_t font_glyph(font_id_t font, unsigned char c, int row)
{
    if (row < 0 || row >= 12) return 0;
    switch (font) {
        default:
        case FONT8X12:
            return font_8x12[c & 0x7F][row];
        case FONT8X12_BOLD:
            return font_8x12_bold[c & 0x7F][row];
    }
}

int font_w(font_id_t font) { (void)font; return 8; }
int font_h(font_id_t font) { (void)font; return 12; }
