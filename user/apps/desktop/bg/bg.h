/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: bg.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#define BG_PATH "/emr/system/desktop/background/1.bmp"

void bg_init(int w, int h);
void bg_draw_full(void);
void bg_draw_rect(int x, int y, int w, int h);