/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: print.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "print.h"
#include "boot.h"
#include "log.h"
#include <kernel/graph/graphics.h>
#include <kernel/kernel_processes/fm/fm.h>
#include <kernel/communication/serial.h>
//#include <ui/fonts/font_8x8.h>

//#include <kernel/console/graph/dos.h>

// made by Dexoron Feb. 18, 2026
// Decode one UTF-8 sequence and return pointer to next byte.
// On invalid data, returns '?' and advances by one byte.
static const char *utf8_next_codepoint(const char *p, u32 *codepoint)
{
    if (!p || !codepoint) return p;

    unsigned char b0 = (unsigned char)p[0];
    if (b0 == 0) {
        *codepoint = 0;
        return p;
    }

    // 1-byte ASCII: 0xxxxxxx
    if ((b0 & 0x80) == 0) {
        *codepoint = b0;
        return p + 1;
    }

    // Continuation byte as lead is invalid.
    if ((b0 & 0xC0) == 0x80) {
        *codepoint = '?';
        return p + 1;
    }

    // 2-byte: 110xxxxx 10xxxxxx
    if ((b0 & 0xE0) == 0xC0) {
        unsigned char b1 = (unsigned char)p[1];
        if ((b1 & 0xC0) != 0x80) {
            *codepoint = '?';
            return p + 1;
        }

        u32 cp = ((u32)(b0 & 0x1F) << 6) | (u32)(b1 & 0x3F);
        // Overlong encoding check: must be >= 0x80
        if (cp < 0x80) {
            *codepoint = '?';
            return p + 1;
        }

        *codepoint = cp;
        return p + 2;
    }

    // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
    if ((b0 & 0xF0) == 0xE0) {
        unsigned char b1 = (unsigned char)p[1];
        unsigned char b2 = (unsigned char)p[2];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) {
            *codepoint = '?';
            return p + 1;
        }

        u32 cp = ((u32)(b0 & 0x0F) << 12) |
                 ((u32)(b1 & 0x3F) << 6)  |
                 (u32)(b2 & 0x3F);
        // Overlong and UTF-16 surrogate range checks
        if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) {
            *codepoint = '?';
            return p + 1;
        }

        *codepoint = cp;
        return p + 3;
    }

    // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if ((b0 & 0xF8) == 0xF0) {
        unsigned char b1 = (unsigned char)p[1];
        unsigned char b2 = (unsigned char)p[2];
        unsigned char b3 = (unsigned char)p[3];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) {
            *codepoint = '?';
            return p + 1;
        }

        u32 cp = ((u32)(b0 & 0x07) << 18) |
                 ((u32)(b1 & 0x3F) << 12) |
                 ((u32)(b2 & 0x3F) << 6) |
                 (u32)(b3 & 0x3F);
        // Valid Unicode scalar range: U+10000..U+10FFFF
        if (cp < 0x10000 || cp > 0x10FFFF) {
            *codepoint = '?';
            return p + 1;
        }

        *codepoint = cp;
        return p + 4;
    }

    *codepoint = '?';
    return p + 1;
}


static u32 g_dirty_y0 = 0xFFFFFFFFu;
static u32 g_dirty_y1 = 0;

static void dirty_reset(void)
{
    g_dirty_y0 = 0xFFFFFFFFu;
    g_dirty_y1 = 0;
}
static void dirty_mark(u32 y, u32 h)
{
    if (y < g_dirty_y0) g_dirty_y0 = y;
    u32 end = y + h;
    if (end > g_dirty_y1) g_dirty_y1 = end;
}
static void dirty_flush(void)
{
    if (g_dirty_y0 > g_dirty_y1) return;
    bs_flush_rows(g_dirty_y0, g_dirty_y1 - g_dirty_y0);
    dirty_reset();
}

#define BS_GLYPH_ROW_MAX 64


static u32 g_scanline[BS_GLYPH_ROW_MAX];

static void putchar_at(u32 codepoint, u32 x, u32 y, u32 color)
{
    u32 char_width  = fm_get_char_width();
    u32 char_height = fm_get_char_height();
    u32 row_bytes   = fm_get_glyph_row_bytes();
    u32 lsb_left    = fm_get_glyph_lsb_left();
    //u32 pitch_dwords = fb_pitch / 4;
    u32 bg_color    = bg();

    const u8 *glyph = fm_get_glyph_cp(codepoint);
    if (!glyph) return;

    u32 pdw     	= bs_backbuf_pitch_dw();
    u32 *backbuf = bs_backbuf_get();
    u32 glyph_w 	= char_width * font_scale;
    u32 glyph_h 	= char_height * font_scale;

    if (glyph_w > BS_GLYPH_ROW_MAX) glyph_w = BS_GLYPH_ROW_MAX;

    bs_screen_t *scr = bs_get_active();

    for (u32 dy = 0; dy < char_height; dy++)
    {
        u32 row_bits = 0;
        const u8 *row_ptr = glyph + (dy * row_bytes);
        if (row_bytes == 1) {
            row_bits = row_ptr[0];
        } else if (row_bytes == 2) {
            row_bits = (u32)row_ptr[0] | ((u32)row_ptr[1] << 8);
        } else if (row_bytes == 4) {
            row_bits = (u32)row_ptr[0] | ((u32)row_ptr[1] << 8) |
                       ((u32)row_ptr[2] << 16) | ((u32)row_ptr[3] << 24);
        } else {
            return;
        }


        for (u32 dx = 0; dx < char_width; dx++)
        {
            u32 bit_index   	= lsb_left ? dx : ((char_width - 1u) - dx);
            u32 pixel_color 	= (row_bits & (1u << bit_index)) ? color : bg_color;
            u32 base        	= dx * font_scale;
            for (u32 sx = 0; sx < font_scale; sx++) g_scanline[base + sx] = pixel_color;
        }
        for (u32 sy = 0; sy < font_scale; sy++)
        {
            u32 abs_y = y + dy * font_scale + sy;
            if (abs_y >= scr->height) break;
            u32 copy_w = glyph_w;
            if (x + copy_w > scr->width) {
                if (x >= scr->width) break;
                copy_w = scr->width - x;
            }
            // write at local coords into the screen's own buffer
            memcpy(
            	backbuf + abs_y * pdw + x,
                g_scanline,
                copy_w * sizeof(u32)
            );
        }
    }

    /* shouldnt flush*/
    dirty_mark(y, glyph_h);
}

static void bs_scroll(bs_screen_t *scr, u32 line_height)
{
    u32 pdw    = scr->width; // local buffer stride
    u32 *buf   = scr->buffer;
    u32 width  = scr->width;
    u32 height = scr->height;

    if (!buf || !pdw) return;

    for (u32 y = 0; y < height - line_height; y++)
    {
        memcpy(
            buf + y * pdw,
            buf + (y + line_height) * pdw,
            width * sizeof(u32)
        );
    }
    for (u32 y = height - line_height; y < height; y++)
    {
        memset(buf + y * pdw, 0, width * sizeof(u32));
    }

    bs_flush_rect(0, 0, width, height);
    //bs_backbuf_flush_all();

    dirty_reset();
}

static void putcodepoint(u32 codepoint, u32 color)
{
	bs_screen_t *src = bs_get_active();
	u32 region_w = src->width;
	u32 region_h = src->height;
    u32 char_width  = fm_get_char_width() * font_scale;
    u32 char_height = fm_get_char_height() * font_scale;
    u32 char_spacing = char_width;
    u32 line_height = char_height + 2 * font_scale;

    //log_printf(d, "CURSOR", "BEFORE y=%d\n", src->cursor_y);

    if (codepoint == '\n')
    {
        src->cursor_x = 0;
        src->cursor_y += line_height;
        // scroll only once, right here
        if (src->cursor_y + char_height > region_h) {
        	bs_scroll(src, line_height);
            src->cursor_y -= line_height;
        }
        //log_printf(d, "CURSOR", "AFTER y=%d\n", src->cursor_y);
        return;
    }

    if (src->cursor_x + char_width >= region_w)
    {
        bs_get_active()->cursor_x = 0;
        bs_get_active()->cursor_y += line_height;
        // scroll only once, right here
        if (src->cursor_y + char_height > region_h) {
        	bs_scroll(src, line_height);
            bs_get_active()->cursor_y -= line_height;
        }
    }

    // removed the third redundant scroll check that was here before drawing
    putchar_at(codepoint, bs_get_active()->cursor_x, bs_get_active()->cursor_y, color);
    bs_get_active()->cursor_x += char_spacing;
}

/*static void aware_scroll_byprint(void)
{
    u32 char_height = fm_get_char_height() * font_scale;
    u32 fb_h = get_fb_height();

    // Check if cursor is out of screen
    if (cursor_y + char_height > fb_h) {
        u32 line_height = char_height + 2 * font_scale;

        // Use scroll_up from graphics.c which is already theme-aware
        scroll_up(line_height);

        // Move cursor up
        cursor_y -= line_height;
        if (cursor_y < 0) {
            cursor_y = 0;
        }
    }
}*/

void putchar(char c, u32 color)
{
    putcodepoint((u32)(unsigned char)c, color);

    //dirty_flush();
}

void string(const char *str, u32 color)
{
	bs_log_write(bs_active, str);
    dirty_reset();
    const char *p = str;
    while (p && *p) {
        u32 cp = 0;
        p = utf8_next_codepoint(p, &cp);
        if (cp == 0) break;
        putcodepoint(cp, color);
    }
    /* entire string*/
    /*
     * TODO:
     * copy and move multiple lines up when screen is ful
     */
    dirty_flush();
    printf("%s", str); // prints everything from the os terminal to the host-terminal
}

void IntToString(int value, char *buffer)
{
    char temp[11];
    int i = 0;
    int isNegative = 0;

    if (value < 0)
    {
        isNegative = 1;
        value = -value;
    }

    do
    {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);

    if (isNegative)
    {
        temp[i++] = '-';
    }

    temp[i] = '\0';

    for (int j = 0; j < i; j++)
    {
        buffer[j] = temp[i - j - 1];
    }

    buffer[i] = '\0';
}


void print_to(int screen, const char *str, u32 color)
{
	#if BOOTUP_VISUALS == 0
		int old = bs_active;
		bs_active = screen;
		string(str,color);
		bs_active = old;
	#endif
}

void printInt_to(int screen, int value, u32 color)
{
	#if BOOTUP_VISUALS == 0
	    char buffer[12];
	    IntToString(value, buffer);

	    int old = bs_active;
	    bs_active = screen;

	    string(buffer, color);

	    bs_active = old;
    #endif
}

void printInt(int value, u32 color)
{
	#if BOOTUP_VISUALS == 0
	    char buffer[12];
	    IntToString(value, buffer);
	    print_to(SCREENMODE, buffer, color);
	#endif
	//printf("%s", value);
}

void print(const char *str, u32 color)
{
	#if BOOTUP_VISUALS == 0
    	print_to(SCREENMODE, str, color);
     //putchar('\n', color);
    #endif
    //printf("%s", str);
}

void reset_cursor(void)
{
    bs_get_active()->cursor_x = 0;
    bs_get_active()->cursor_y = 0;
}