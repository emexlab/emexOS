/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: win.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include <sys/types.h>
#include "../config/cfg.h"

#define DT_WIN      0x00
#define DT_POPUP    0x01
#define DT_NOMOVE   0x02
#define DT_NOTITLE  0x04

#define DT_WIN_MAX    32
#define DT_TITLE_MAX  64

typedef struct {
    pid_t pid;
    char title[DT_TITLE_MAX];
    int x, y, w, h;
    unsigned int style;
    int valid;
    int focused;
    int z;

    int home_cx, home_cy, home_cw, home_ch;

    int orig_cx, orig_cy, orig_cw, orig_ch;

    unsigned int *buf;

    int buf_w;
    int buf_h;
} dt_win_t;

int win_add(
    pid_t pid, const char *title,
    int x, int y, int w, int h, unsigned int style
);
void win_remove(pid_t pid);
void win_set_title(pid_t pid, const char *title);
void win_move(int idx, int nx, int ny);
void win_focus(int idx);
void win_update_buf(int idx, const unsigned int *pixels, int w, int h);

int win_find_pid(pid_t pid);
int win_hit(int idx, int mx, int my);
int win_hit_title(int idx, int mx, int my);
int win_hit_close(int idx, int mx, int my);

dt_win_t *win_get(int idx);
int win_count(void);