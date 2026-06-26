/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: libdesktop.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include "dt_input.h"

#define DT_WIN     0x00
#define DT_POPUP   0x01
#define DT_NOMOVE  0x02
#define DT_NOTITLE 0x04

#define DT_TITLE_H 18
#define DT_BORDER  1

typedef struct {
    int x, y, w, h;
} DesktopArea;

static inline DesktopArea desktopContentArea(
    int x, int y, int w, int h, unsigned int style)
{
    DesktopArea a;
    if (style & DT_POPUP) {
        a.x = x + 1; a.y = y + 1;
        a.w = w - 2; a.h = h - 2;
    } else if (style & DT_NOTITLE) {
        a.x = x + DT_BORDER;     a.y = y + DT_BORDER;
        a.w = w - DT_BORDER * 2; a.h = h - DT_BORDER * 2;
    } else {
        // 2px border + 18px titlebar + 1px separator
        a.x = x + DT_BORDER;
        a.y = y + DT_TITLE_H + 1;
        a.w = w - DT_BORDER * 2;
        a.h = h - DT_TITLE_H - 1 - DT_BORDER;
    }
    return a;
}

typedef struct {

    // register
    int  (*createWindow)(
        const char *title,
        int x, int y, int w, int h,
        unsigned int style
    );
    int (*createPopup)(
    	int x,
     	int y,
      	int w,
       	int h
    );

    // unregister
    void (*closeWindow)(void);
    void (*setTitle)(const char *title);
    void (*markDirty)(void);
    void (*winbuf_write)(const unsigned int *pixels, int w, int h);
    int (*pollEvents)(dt_event_t *buf, int max);
    void (*getCurrentMousePos)(int *out_x, int *out_y);

} Desktop;

extern Desktop desktop;