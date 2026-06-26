/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: exui.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include "../libfont/libfont.h"

#define EXUI_INPUTBOX_MAX 256
#define EXUI_MENU_MAX 32

typedef struct {
    char buf[EXUI_INPUTBOX_MAX];

    int 	len;
    int 	focused;
    int 	mask;
} exui_inputbox_t;

typedef struct {
    int 	hovered;
    int 	pressed;
} exui_btn_state_t;

/*
 *
 * TODO:
 * finish psf loading at exui.Print(..., "text", ..., "path/to/psf_font.psf");
typedef enum
{
	EXUI_FONT_BUILTIN,
	EXUI_FONT_PSF
}exui_font_type_t;

typedef struct psf_font psf_font_t;

typedef struct
{
	exui_font_type_t type;
	union
	{
		font_id_t builtin;
		psf_font_t *psf;
	};
} exui_font_t
*/

typedef struct {
    const char **items;
    int count;
    int hover;   // which item is hovered right now
} exui_menu_t;

typedef struct
{
    void (*DrawRectangle)(int x, int y, int w, int h, unsigned int color);
    void (*DrawPixel) (int x, int y, unsigned int color);
    void (*DrawCursor) (int x, int y, unsigned int color, font_id_t font);
    void (*Print) (int x, int y, const char *text, unsigned int fg, unsigned int bg, font_id_t font);

    void (*Clear)(unsigned int color);
    void (*Flush)(void);

    int  (*InputBox)(
    	int x, int y, int w, exui_inputbox_t *state,
        unsigned int fg, unsigned int bg, unsigned int border,
        font_id_t font, unsigned int keycode
    );

    void (*BlitPixels)(int x, int y, int img_w, int img_h, const unsigned int *pixels, int dst_w, int dst_h);

    // menus
    int (*MenuDraw)(exui_menu_t *m, int mx, int my, int btn_clicked);
    void (*MenuSize)(int item_count, int *out_w, int *out_h);
    void (*HSep)(int x, int y, int w, unsigned int color);
    int  (*Button)(
    	int x, int y, int w, int h,
        const char *label, font_id_t font,
        int mx, int my, int btn_down, exui_btn_state_t *state
    );
    int  (*IconButton)(
    	int x, int y, int w, int h,
        const unsigned int *pixels, int img_w, int img_h,
        int mx, int my, int btn_down, exui_btn_state_t *state
    );

} eXui;

extern eXui exui; // exui.

void exui_init(int w, int h);
int exui_width(void);
int exui_height(void);