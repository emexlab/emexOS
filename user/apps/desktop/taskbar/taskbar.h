/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: taskbar.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

void taskbar_init(int scr_w, int scr_h);
void taskbar_draw(int mx, int my, int btn_down);
int taskbar_click(int mx, int my);
int taskbar_y(void);