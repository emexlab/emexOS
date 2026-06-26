/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: template.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <unistd.h>
#include <stdio.h>

#include "libdesktop.h"
#include "exui.h"

#define APP_TITLE "App Template"
#define WIN_W 400
#define WIN_H 200
#define FONT FONT8X12 // or FONT8X12_BOLD
//                 AARRGGBB
#define BG_COLOR 0xFFFFFFFFu
#define FG_COLOR 0xFF000000u

static DesktopArea ca;

static void draw_ui(void)
{
    exui.Clear(BG_COLOR);

    exui.Print(
        20,
        20,
        "AppTemplate :3",
        FG_COLOR,
        BG_COLOR,
        FONT
    );
    exui.Print(
        20,
        40,
        "this is your app template",
        FG_COLOR,
        BG_COLOR,
        FONT
    );

    // example horizontal seperator
    // (theres no vertical for now but soon :)
    exui.HSep(
        20, // x
        65, // y
        250, // w
        0xFF808080u // color
    );

    exui.Print(
        20,
        85,
        "Place whatever UI you want, like rectangles, lines etc",
        FG_COLOR,
        BG_COLOR,
        FONT
    );

    exui.Flush(); // flush always at least or in a loop when theres something that changes
}

int main(void)
{
    int screen_w = 1280;
    int screen_h = 720;
    int win_x = (screen_w - WIN_W) / 2;
    int win_y = (screen_h - WIN_H) / 2;

    desktop.createWindow(
        APP_TITLE,
        win_x,
        win_y,
        WIN_W,
        WIN_H,
        DT_WIN
    );
    ca = desktopContentArea(
        win_x,
        win_y,
        WIN_W,
        WIN_H,
        DT_WIN
    );

    exui_init(ca.w, ca.h);
    draw_ui();

    return 0;
}