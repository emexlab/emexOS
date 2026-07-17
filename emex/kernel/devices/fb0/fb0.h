/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * emexOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * emexOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with emexOS. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DEVICE_FB0_H
#define DEVICE_FB0_H

#include <kernel/module/module.h>
#include <types.h>

#include <config/system.h>

#define FBN EMEX "fb0"

// fb_var_screeninfo
typedef struct
{
    u32 	xres;
    u32 	yres;
    u32 	xres_virtual;
    u32 	yres_virtual;
    u32 	xoffset;
    u32 	yoffset;
    u32 	bits_per_pixel;
    u32 	grayscale;
    u32 	red_offset;
    u32 	red_length;
    u32 	green_offset;
    u32 	green_length;
    u32 	blue_offset;
    u32 	blue_length;
    u32 	transp_offset;
    u32 	transp_length;
} fb_var_screeninfo_t;
typedef struct
{
    char 	id[16];
    u64 	smem_start;
    u32 	smem_len;
    u32 	type;
    u32 	visual;
    u32 	line_length;   // pitch
} fb_fix_screeninfo_t;

// /dev/fb0 request codes
#define FBIOGET_VSCREENINFO 0x4600
#define FBIOPUT_VSCREENINFO 0x4601
#define FBIOGET_FSCREENINFO 0x4602
#define FBIO_RESET_POS      0x4603
#define FBIO_READ_RECT      0x4610
#define FBIO_BLIT           0x4611

typedef struct {
    u32 	 x, y, w, h;
    u32 	*pixels;
} fb_rect_t;

// /dev/fb0 module
extern driver_module fb0_module;


int fb0_ioctl(int request, void *arg);

#endif
