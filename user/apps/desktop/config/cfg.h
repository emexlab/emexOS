/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: cfg.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#define DT_DIR    "/tmp/dt"
#define DT_CMD    "/tmp/dt/cmd"
#define DT_DIRTY  "/tmp/dt/dirty"
#define DT_CURSOR "/tmp/dt/cursor"
#define MOUSE_DEV "/dev/input/mouse0"

#define DT_BG 0xFF008080u  // win95/98 teal ig

#define DARK_MODE 1

// rubber band selection colors
#define BAND_BORDER 0xCCA8A5A5u   // border
#define BAND_FILL   0x33A8A5A5u   // the content

//taskbar
#define TB_H 34
#define TB_BTN_PAD 2
#define TB_BTN_VPAD 6
#define TB_BTN_W 80
#define TB_BORDER_W 3
#define TB_ENTRY_NAMELEN 32
#define TB_ENTRY_EXECLEN 64
#define TB_MAX_ENTRIES 10

#if DARK_MODE == 1
	#define TB_BACKGROUND  0xFF202020u
	#define TB_TOP_BORDER  0xFF707070u
	#define TB_BUTTON_BG   0xFF404040u
	#define TB_BLACK       0xFF000000u
	#define TB_WHITE       0xFFFFFFFFu
	#define TB_LIGHT       0xFF808080u
	#define TB_SHADOW      0xFF606060u
	#define TB_BTN_TOP     0xFF6A89A7u
	#define TB_FACE        0xFFD4D0C8u
	#define TB_TRANSPARENT 0x00FFFFFFu

	#define WIN_BLACK           0xFF000000u
	#define WIN_WHITE           0xFF404040u
	#define WIN_FACE            0xFFD4D0C8u
	#define WIN_BUTTON          0xFFD4D0C8u
	#define WIN_FOCUSED_BG      0xFF5F5F5Fu
	#define WIN_UNFOCUSED_BG    0xFF5F5F5Fu
#elif DARK_MODE == 0
	#define TB_BACKGROUND  0xFFDFDFDFu
	#define TB_TOP_BORDER  0xFF8F8F8Fu
	#define TB_BUTTON_BG   0xFFBFBFBFu
	#define TB_BLACK       0xFFFFFFFFu
	#define TB_WHITE       0xFF000000u
	#define TB_LIGHT       0xFF7F7F7Fu
	#define TB_SHADOW      0xFF9F9F9Fu
	#define TB_BTN_TOP     0xFF957658u
	#define TB_FACE        0xFF2B2F37u
	#define TB_TRANSPARENT 0x00000000u

	#define WIN_BLACK           0xFF000000u
	#define WIN_WHITE           0xFFFFFFFFu
	#define WIN_FACE            0xFFD4D0C8u
	#define WIN_BUTTON          0xFFD4D0C8u
	#define WIN_FOCUSED_BG      0xFFA0A0A0u
	#define WIN_UNFOCUSED_BG    0xFF000000u
#endif

#define DT_ENABLE_RESIZING 0

// window title bar
#define DT_TITLE_ACT 0xFFB0D0C0u  // green; when focused
#define DT_TITLE_INA 0xFF808080u  // grey ; when unfocused
//#define DT_TITLE_INA 0xFF4F2F3Fu //red
#define DT_TITLE_TXT 0xFFFFFFFFu  // text of titlebar

// titlebar config
#define DT_TITLE_H  18 // tot. inc. high
#define DT_TITLE_PB 2  // pad. top
#define DT_BORDER   1  // border width

#define DT_CLOSE_X  7  // x offset
#define DT_CLOSE_Y  5  // y offset
#define DT_CLOSE_SZ 8 // square size

// font rendering
#define DT_FW 8
#define DT_FH 12