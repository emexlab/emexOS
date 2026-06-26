/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: exui.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "exui.h"
#include "libdesktop.h"
#include "libfont.h"
#include <stdlib.h>
#include <string.h>

static unsigned int *g_buf = NULL;
static int g_w = 0;
static int g_h = 0;

void exui_init(int w, int h)
{
    if (g_buf) free(g_buf);
    if (w <= 0 || h <= 0) return;

    g_buf = NULL;
    g_w = 0;
    g_h = 0;

    g_buf = (unsigned int *)malloc((unsigned)(w * h * 4));
    if (!g_buf) return;

    g_w = w;
    g_h = h;
    memset(g_buf, 0, (unsigned)(w * h * 4));
}

int exui_width(void)  { return g_w; }
int exui_height(void) { return g_h; }

static inline void _set(int x, int y, unsigned int c)
{
    if (!g_buf || x < 0 || x >= g_w || y < 0 || y >= g_h) return;
    g_buf[y * g_w + x] = c;
}

static void _DrawRectangle(int x, int y, int w, int h, unsigned int color)
{
    for (int dy = 0; dy < h; dy++)
        for (int dx = 0; dx < w; dx++)
            _set(x + dx, y + dy, color);
}

static void _DrawPixel(int x, int y, unsigned int color)
{
    _set(x, y, color);
}

static void _Print(
	int x, int y, const char *text,
	unsigned int fg, unsigned int bg,
	font_id_t font
){
    if (!g_buf || !text) return;
    int fw = font_w(font);
    int fh = font_h(font);
    int px = x;
    while (*text)
    {
        unsigned char c = (unsigned char)*text & 0x7Fu;
        for (int row = 0; row < fh; row++)
        {
            int ay = y + row;
            if (ay < 0 || ay >= g_h) continue;
            uint16_t bits = font_glyph(font, c, row);
            for (int col = 0; col < fw; col++)
            {
                int ax = px + col;
                if (ax < 0 || ax >= g_w) continue;
                g_buf[ay * g_w + ax] = (bits & (1u << col)) ? fg : bg;
            }
        }
        text++;
        px += fw;
    }
}

static void _DrawCursor(int x, int y, unsigned int color, font_id_t font)
{
    _DrawRectangle(x, y, 2, font_h(font), color);
}

static int _InputBox(
	int x, int y, int w, exui_inputbox_t *state,
    unsigned int fg, unsigned int bg, unsigned int border,
    font_id_t font, unsigned int keycode
){
    if (!state) return 0;
    int fw = font_w(font);
    int fh = font_h(font);
    int h = fh + 6;
    int committed = 0;

    if (keycode != 0)
    {
        char c = (char)(keycode & 0xFF);
        if (keycode == '\n' || keycode == '\r') { committed = 1; }
        else if (keycode == '\b') { if (state->len > 0) state->buf[--state->len] = '\0'; }
        else if (c >= 0x20 && c <= 0x7E && state->len < EXUI_INPUTBOX_MAX - 1)
        {
            state->buf[state->len++] = c;
            state->buf[state->len]   = '\0';
        }
    }

    _DrawRectangle(x, y,     w, h, bg);
    _DrawRectangle(x, y,     w, 1, border);
    _DrawRectangle(x, y+h-1, w, 1, border);
    _DrawRectangle(x, y,     1, h, border);
    _DrawRectangle(x+w-1, y, 1, h, border);

    if (state->len > 0) {
        char display[EXUI_INPUTBOX_MAX];

        if (state->mask)
        {
            for (int i = 0; i < state->len; i++) display[i] = '*';
            display[state->len] = '\0';
        } else {
            for (int i = 0; i <= state->len; i++) display[i] = state->buf[i];
        }
        int max_chars = (w - 6) / (fw > 0 ? fw : 1);
        int start = state->len > max_chars ? state->len - max_chars : 0;
        _Print(x + 3, y + 3, display + start, fg, bg, font);
    }

    if (state->focused)
    {
        int max_chars = (w - 6) / (fw > 0 ? fw : 1);
        int clipped   = state->len > max_chars ? max_chars : state->len;
        _DrawCursor(x + 3 + clipped * fw, y + 3, fg, font);
    }

    return committed;
}

#define _BTN_FACE   0xFFD4D0C8u
#define _BTN_LIGHT  0xFFFFFFFFu
#define _BTN_SHADOW 0xFF808080u
#define _BTN_DARK   0xFF404040u
#define _BTN_HOVER  0xFFDCDCDCu
#define _BTN_FG     0xFF000000u

static int _rect_hit(int rx, int ry, int rw, int rh, int mx, int my)
{
    return mx >= rx && mx < rx + rw && my >= ry && my < ry + rh;
}

static int _Button(
	int x, int y, int w, int h,
    const char *label, font_id_t font,
    int mx, int my, int btn_down, exui_btn_state_t *state
){
    int hit     = _rect_hit(x, y, w, h, mx, my);
    int pressed = hit && btn_down;
    if (state) { state->hovered = hit; state->pressed = pressed; }

    unsigned int face = hit ? _BTN_HOVER : _BTN_FACE;
    _DrawRectangle(x, y, w, h, face);
    if (pressed) {
        _DrawRectangle(x,     y,     w, 1, _BTN_DARK);
        _DrawRectangle(x,     y,     1, h, _BTN_DARK);
        _DrawRectangle(x,     y+h-1, w, 1, _BTN_LIGHT);
        _DrawRectangle(x+w-1, y,     1, h, _BTN_LIGHT);
    } else {
        _DrawRectangle(x,     y,     w,   1, _BTN_LIGHT);
        _DrawRectangle(x,     y,     1,   h, _BTN_LIGHT);
        _DrawRectangle(x,     y+h-2, w,   1, _BTN_SHADOW);
        _DrawRectangle(x+1,   y+h-1, w-1, 1, _BTN_DARK);
        _DrawRectangle(x+w-2, y+1,   1, h-2, _BTN_SHADOW);
        _DrawRectangle(x+w-1, y,     1,   h, _BTN_DARK);
    }
    if (label && label[0])
    {
        int lw = 0;
        for (int i = 0; label[i]; i++) lw += font_w(font);
        int tx = x + (w - lw) / 2 + (pressed ? 1 : 0);
        int ty = y + (h - font_h(font)) / 2 + (pressed ? 1 : 0);
        _Print(tx, ty, label, _BTN_FG, face, font);
    }
    return pressed;
}

static int _IconButton(
	int x, int y, int w, int h,
    const unsigned int *pixels, int img_w, int img_h,
    int mx, int my, int btn_down, exui_btn_state_t *state
){
    int hit     	= _rect_hit(x, y, w, h, mx, my);
    int pressed 	= hit && btn_down;

    if (state) {
    	state->hovered = hit; state->pressed = pressed;
    }

    unsigned int face = hit ? _BTN_HOVER : _BTN_FACE;
    _DrawRectangle(x, y, w, h, face);

    if (pressed) {
        _DrawRectangle(x,     y,     w, 1, _BTN_DARK);
        _DrawRectangle(x,     y,     1, h, _BTN_DARK);
        _DrawRectangle(x,     y+h-1, w, 1, _BTN_LIGHT);
        _DrawRectangle(x+w-1, y,     1, h, _BTN_LIGHT);
    } else {
        _DrawRectangle(x,     y,     w,   1, _BTN_LIGHT);
        _DrawRectangle(x,     y,     1,   h, _BTN_LIGHT);
        _DrawRectangle(x,     y+h-2, w,   1, _BTN_SHADOW);
        _DrawRectangle(x+1,   y+h-1, w-1, 1, _BTN_DARK);
        _DrawRectangle(x+w-2, y+1,   1, h-2, _BTN_SHADOW);
        _DrawRectangle(x+w-1, y,     1,   h, _BTN_DARK);
    }
    if (pixels && img_w > 0 && img_h > 0)
    {
        int off = pressed ? 1 : 0;
        int ix  = x + (w - img_w) / 2 + off;
        int iy  = y + (h - img_h) / 2 + off;

        for (int dy = 0; dy < img_h; dy++)
        {
	        for (int dx = 0; dx < img_w; dx++)
			{
	            unsigned int px = pixels[dy * img_w + dx];
	            if ((px >> 24) == 0) continue;
	            _set(ix + dx, iy + dy, px);
	        }
        }
    }
    return pressed;
}

static void _HSep(int x, int y, int w, unsigned int color)
{
    _DrawRectangle(x, y, w, 1, color);
}

static void _BlitPixels(
	int x, int y, int img_w, int img_h,
    const unsigned int *pixels, int dst_w, int dst_h
){
    if (!pixels || img_w <= 0 || img_h <= 0) return;
    int dw = dst_w > 0 ? dst_w : img_w;
    int dh = dst_h > 0 ? dst_h : img_h;

    for (int dy = 0; dy < dh; dy++)
    {
        int sy = (dh > 1) ? (dy * (img_h - 1) / (dh - 1)) : 0;
        if (sy >= img_h) sy = img_h - 1;

        for (int dx = 0; dx < dw; dx++)
        {
            int sx = (dw > 1) ? (dx * (img_w - 1) / (dw - 1)) : 0;
            if (sx >= img_w) sx = img_w - 1;

            unsigned int px = pixels[sy * img_w + sx];

            if ((px >> 24) == 0) continue;
            _set(x + dx, y + dy, px);
        }
    }
}

// menu visuals
#define _MNU_BG      0xFFD4D0C8u
#define _MNU_SEL     0xFF000080u
#define _MNU_SEL_TXT 0xFFFFFFFFu
#define _MNU_FG      0xFF000000u
#define _MNU_SEP     0xFF808080u
#define _MNU_ROW_H   18
#define _MNU_PAD_X   6

static void _MenuSize(int item_count, int *out_w, int *out_h)
{
    *out_w = 140 + 2;
    *out_h = item_count * _MNU_ROW_H + 2;
}
static int _MenuDraw(exui_menu_t *m, int mx, int my, int btn_clicked)
{
    if (!m || !m->items || m->count <= 0) return -1;

    int cw = g_w;
    int ch = g_h;

    // fills background
    _DrawRectangle(0, 0, cw, ch, _MNU_BG);
    /*
     * normally the popup background shouldnt be changable by the app
     * only per themes ig
     */

    int fw = font_w(FONT8X12);
    int fh = font_h(FONT8X12);
    int result = -1;

    m->hover = -1;

    for (int i = 0; i < m->count; i++) {
        int ry = i * _MNU_ROW_H;
        if (ry + _MNU_ROW_H > ch) break;

        // check if mouse is over this row
        int hov = (mx >= 0 && mx < cw && my >= ry && my < ry + _MNU_ROW_H);
        if (hov) m->hover = i;

        unsigned int row_bg 	= hov ? _MNU_SEL     : _MNU_BG;
        unsigned int txt_c  	= hov ? _MNU_SEL_TXT : _MNU_FG;

        _DrawRectangle(0, ry, cw, _MNU_ROW_H, row_bg);

        // separator item: just a horizontal line, label is "-"
        if (m->items[i][0] == '-' && m->items[i][1] == '\0') {
            int sy = ry + _MNU_ROW_H / 2;
            _DrawRectangle(4, sy, cw - 8, 1, _MNU_SEP);
            continue;
        }

        int ty = ry + (_MNU_ROW_H - fh) /  2;

        _Print(_MNU_PAD_X, ty, m->items[i], txt_c, row_bg, FONT8X12);

        if (hov && btn_clicked) result = i;
    }

    // click outside the menu area closes it
    if (btn_clicked && result < 0)
    {
        int in_menu = (mx >= 0 && mx < cw && my >= 0 && my < ch);
        if (!in_menu) return -2;
    }

    return result;
}

static void _Clear(unsigned int color)
{
    if (!g_buf) return;
    int total = g_w * g_h;
    for (int i = 0; i < total; i++) g_buf[i] = color;
}

static void _Flush(void)
{
    if (!g_buf || g_w <= 0 || g_h <= 0) return;
    desktop.winbuf_write(g_buf, g_w, g_h);
}

eXui exui = {
    .DrawRectangle = _DrawRectangle,
    .DrawPixel     = _DrawPixel,
    .Print         = _Print,
    .DrawCursor    = _DrawCursor,
    .InputBox      = _InputBox,
    .Button        = _Button,
    .IconButton    = _IconButton,
    .HSep          = _HSep,
    .BlitPixels    = _BlitPixels,
    .MenuDraw      = _MenuDraw,
    .MenuSize      = _MenuSize,
    .Clear         = _Clear,
    .Flush         = _Flush,
};