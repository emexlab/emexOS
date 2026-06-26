/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: desktop_input_dispatch.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include <sys/types.h>
#include "../../libs/libdesktop/dt_input.h"
#include "win/win.h"

void dt_set_screen_size(int w, int h);

void dt_dispatch_event(pid_t focused_pid, const dt_event_t *ev);
void dt_clear_input(pid_t pid);

int dt_make_mouse_event(int focused_idx, int abs_x, int abs_y, unsigned char buttons, dt_event_t *out);
int dt_make_key_event(unsigned int keycode, unsigned char modifiers,unsigned char pressed, dt_event_t *out);

void dt_clamp_popup(int *x, int *y, int w, int h);