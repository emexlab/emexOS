/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: welcome.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <unistd.h>
#include <stdio.h>

#include "libdesktop.h"
#include "exui.h"

#define APP_TITLE "Welcome"
#define WIN_W 500
#define WIN_H 250

#define BG_COLOR 0xFFFFFFFFu
#define FG_COLOR 0xFF000000u
#define ACCENT_COLOR 0xFF0066CCu

static DesktopArea ca;

static void draw_ui(void)
{
    exui.Clear(BG_COLOR);

    exui.Print(
        20,
        20,
        "Welcome to emexOS!",
        ACCENT_COLOR,
        BG_COLOR,
        FONT8X12_BOLD
    );

    exui.HSep(
        20,
        35,
        450,
        0xFF808080u
    );

    exui.Print(
        20,
        45,
        "emexOS is a small graphical operating system entirely",
        FG_COLOR,
        BG_COLOR,
        FONT8X12
    );

    exui.Print(
        20,
        65,
        "written from scratch in C by emex and all its contributors.",
        FG_COLOR,
        BG_COLOR,
        FONT8X12
    );

    exui.Print(
        20,
        85,
        "It's designed for customizers who want to fully change the",
        FG_COLOR,
        BG_COLOR,
        FONT8X12
    );

    exui.Print(
        20,
        105,
        "look of their system and love retro with a modern touch.",
        FG_COLOR,
        BG_COLOR,
        FONT8X12
    );

    exui.Print(
        20,
        140,
        "Have fun exploring the system!",
        FG_COLOR,
        BG_COLOR,
        FONT8X12
    );

    exui.Print(
        WIN_W - 24 - (15 * 8),
        185,
        "~emex-foundation ",
        ACCENT_COLOR,
        BG_COLOR,
        FONT8X12_BOLD
    );

    exui.Flush();
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