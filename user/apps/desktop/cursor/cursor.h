/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: cursor.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include "data.h"

// cursor type enum
typedef enum {
    CUR_TYPE_NORMAL     		= 0,
    CUR_TYPE_HRESIZE    		= 1,
    CUR_TYPE_VRESIZE    		= 2,
    CUR_TYPE_DRESIZE_NWSE 		= 3,
    CUR_TYPE_DRESIZE_NESW 		= 4,
} cur_type_t;

void cur_init(int fb_fd, int w, int h);
void cur_undo_from_backbuf(void);
void cur_bake(int x, int y);
void cur_erase_fb(void);
void cur_draw_fb(int x, int y);
void cur_set_type(cur_type_t type);

int cur_valid(void);
int cur_w(void);
int cur_h(void);