/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
 *
 * Private use and modification are permitted under the license.
 * Redistribution of source or binaries, modified or unmodified,
 * requires prior written permission from emexLabs.
 *
 * External deployment is subject to the source-availability
 * requirements and other conditions of the license.
 *
 * Provided "AS IS", without warranty, to the extent permitted by law.
 *
 * See LICENSE for the full license terms.
 */

#include <fw.h>
#include <display.h>

u16 gui_line, gui_column;

static void gui_scroll_up(u64 lines)
{
    u64 shift = lines * gui_width;
    u64 rest = (gui_height - lines) * gui_width;
    memmove((void *)gui_fb, (void *)(gui_fb + shift), rest);
    memset((void *)(gui_fb + rest), 0, shift);
}

static void gui_draw_glyph(unsigned char c,
                           u64 x,
                           u64 y)
{
    const u8 *glyph = &iso_font[c * GUI_GLYPH_BYTES];
    for(u64 row = 0; row < GUI_GLYPH_BYTES && y + row < gui_height; row++)
    {
        u8 *line = (u8 *)(gui_fb + (y + row) * gui_width);
        for(u64 col = 0; col < 8 && x + col < gui_width; col++)
        {
            line[x + col] = (glyph[row] >> col) & 1 ? FB_TEXT_COLOR : 0;
        }
    }
}

void gui_putc(int c)
{
    unsigned char ch = (unsigned char)c;
    if(ch == 0)
    {
        return;
    }

    if(ch == '\n')
    {
        gui_column = 0;
        if(gui_line + 1 >= gui_height / GUI_LINE_HEIGHT)
        {
            gui_scroll_up(GUI_LINE_HEIGHT);
        }
        else
        {
            gui_line++;
        }
        return;
    }
    if(ch == '\r')
    {
        gui_column = 0;
        return;
    }

    u64 x = (u64)gui_column * 8 + GUI_MARGIN;
    u64 y = (u64)gui_line * GUI_LINE_HEIGHT + GUI_MARGIN;
    gui_column++;
    gui_draw_glyph(ch, x, y);
}
