/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: data.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#define T 0x00000000u
#define B 0xFF101010u
#define W 0xFFFFFFFFu
#define X 0xFF808080u  // gray for resize arrows

// normal arrow cursor 12x16
static const unsigned int cur_normal_px[12 * 16] = {
    B,T,T,T,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,T,T,T,
    B,W,W,W,B,T,T,T,T,T,T,T,
    B,W,W,W,W,B,T,T,T,T,T,T,
    B,W,W,W,W,W,B,T,T,T,T,T,
    B,W,W,W,W,W,W,B,T,T,T,T,
    B,W,W,W,W,W,W,W,B,T,T,T,
    B,W,W,W,W,W,W,W,W,B,T,T,
    B,W,W,W,W,W,W,W,W,W,B,T,
    B,W,W,W,W,B,B,B,B,B,B,T,
    B,W,W,W,B,T,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,T,T,T,
};

// horizontal resize cursor (left/right) 16x16
static const unsigned int cur_hresize_px[16 * 16] = {
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
	T,T,T,T,T,T,B,T,T,B,T,T,T,T,T,T,
	T,T,T,T,T,B,B,T,T,B,B,T,T,T,T,T,
	T,T,T,T,B,W,B,T,T,B,W,B,T,T,T,T,
	T,T,T,B,W,W,B,T,T,B,W,W,B,T,T,T,
	T,T,B,W,W,W,B,T,T,B,W,W,W,B,T,T,
	T,B,W,W,W,W,B,B,B,B,W,W,W,W,B,T,
	B,W,W,W,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,W,W,W,B,
	T,B,W,W,W,W,B,B,B,B,W,W,W,W,B,T,
	T,T,B,W,W,W,B,T,T,B,W,W,W,B,T,T,
	T,T,T,B,W,W,B,T,T,B,W,W,B,T,T,T,
	T,T,T,T,B,W,B,T,T,B,W,B,T,T,T,T,
	T,T,T,T,T,B,B,T,T,B,B,T,T,T,T,T,
	T,T,T,T,T,T,B,T,T,B,T,T,T,T,T,T,
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

// vertical resize cursor (top/bottom) 16x16
static const unsigned int cur_vresize_px[16 * 16] = {
	T,T,T,T,T,T,T,B,B,T,T,T,T,T,T,T,
	T,T,T,T,T,T,B,W,W,B,T,T,T,T,T,T,
	T,T,T,T,T,B,W,W,W,W,B,T,T,T,T,T,
	T,T,T,T,B,W,W,W,W,W,W,B,T,T,T,T,
	T,T,T,B,W,W,W,W,W,W,W,W,B,T,T,T,
	T,T,B,W,W,W,W,W,W,W,W,W,W,B,T,T,
	T,B,B,B,B,B,B,W,W,B,B,B,B,B,B,T,
	T,T,T,T,T,T,B,W,W,B,T,T,T,T,T,T,
	T,T,T,T,T,T,B,W,W,B,T,T,T,T,T,T,
	T,B,B,B,B,B,B,W,W,B,B,B,B,B,B,T,
	T,T,B,W,W,W,W,W,W,W,W,W,W,B,T,T,
	T,T,T,B,W,W,W,W,W,W,W,W,B,T,T,T,
	T,T,T,T,B,W,W,W,W,W,W,B,T,T,T,T,
	T,T,T,T,T,B,W,W,W,W,B,T,T,T,T,T,
	T,T,T,T,T,T,B,W,W,B,T,T,T,T,T,T,
	T,T,T,T,T,T,T,B,B,T,T,T,T,T,T,T,
};

// diagonal resize NW/SE 16x16
static const unsigned int cur_dresize_nwse_px[16 * 16] = {
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
	T,B,B,B,B,B,B,B,B,B,B,T,T,T,T,T,
	T,B,W,W,W,W,W,W,W,W,B,T,T,T,T,T,
	T,B,W,W,W,W,W,W,W,B,T,T,T,T,T,T,
	T,B,W,W,W,W,W,W,B,T,T,T,T,T,T,T,
	T,B,W,W,W,W,W,B,T,T,T,T,T,B,B,T,
	T,B,W,W,W,W,W,W,B,T,T,T,B,W,B,T,
	T,B,W,W,W,B,W,W,W,B,T,B,W,W,B,T,
	T,B,W,W,B,T,B,W,W,W,B,W,W,W,B,T,
	T,B,W,B,T,T,T,B,W,W,W,W,W,W,B,T,
	T,B,B,T,T,T,T,T,B,W,W,W,W,W,B,T,
	T,T,T,T,T,T,T,B,W,W,W,W,W,W,B,T,
	T,T,T,T,T,T,B,W,W,W,W,W,W,W,B,T,
	T,T,T,T,T,B,W,W,W,W,W,W,W,W,B,T,
	T,T,T,T,T,B,B,B,B,B,B,B,B,B,B,T,
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,

};

// diagonal resize NE/SW 16x16
static const unsigned int cur_dresize_nesw_px[16 * 16] = {
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
	T,T,T,T,T,B,B,B,B,B,B,B,B,B,B,T,
	T,T,T,T,T,B,W,W,W,W,W,W,W,W,B,T,
	T,T,T,T,T,T,B,W,W,W,W,W,W,W,B,T,
	T,T,T,T,T,T,T,B,W,W,W,W,W,W,B,T,
	T,B,B,T,T,T,T,T,B,W,W,W,W,W,B,T,
	T,B,W,B,T,T,T,B,W,W,W,W,W,W,B,T,
	T,B,W,W,B,T,B,W,W,W,B,W,W,W,B,T,
	T,B,W,W,W,B,W,W,W,B,T,B,W,W,B,T,
	T,B,W,W,W,W,W,W,B,T,T,T,B,W,B,T,
	T,B,W,W,W,W,W,B,T,T,T,T,T,B,B,T,
	T,B,W,W,W,W,W,W,B,T,T,T,T,T,T,T,
	T,B,W,W,W,W,W,W,W,B,T,T,T,T,T,T,
	T,B,W,W,W,W,W,W,W,W,B,T,T,T,T,T,
	T,B,B,B,B,B,B,B,B,B,B,T,T,T,T,T,
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

#undef T
#undef B
#undef W
#undef X

// cursor sizes
#define CUR_NORMAL_W  12
#define CUR_NORMAL_H  16
#define CUR_RESIZE_W  16
#define CUR_RESIZE_H  16