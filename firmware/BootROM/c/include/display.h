/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
 *
 * Private use and modification are permitted under the license.
 * Redistribution of source or binaries, modified or unmodified,
 * requires prior written permission from emexLabs.
 *
 * External deployment is subject to the source-availability
 * requirements and other conditions of the license.
 *
 * Provided "AS IS", without warranty, to the extent permitted by law.
 *
 * See LICENSE for the full license terms.
 */

#ifndef FW_DISPLAY_H
#define FW_DISPLAY_H

#include <fw.h>

#define GPU_PCI_ID              0x0003E640UL

/* display controller registers (BAR0) */
#define GPU_DP_STATUS           0x100
#define GPU_DP_LINK             0x108
#define GPU_DP_AUX_ADDR         0x110
#define GPU_DP_AUX_DATA         0x118
#define GPU_AUX_EDID            0x80000000UL
#define GPU_DP_HPD              1
#define GPU_SCAN_CTRL           0x200
#define GPU_SCAN_BASE           0x208
#define GPU_SCAN_WIDTH          0x210
#define GPU_SCAN_HEIGHT         0x218
#define GPU_SCAN_STRIDE         0x220
#define GPU_SCAN_FORMAT         0x228
#define GPU_FORMAT_I8           1
#define GPU_PALETTE             0x1000

#define FB_TEXT_COLOR           0xFF

#define GUI_LINE_HEIGHT         16
#define GUI_GLYPH_BYTES         16
#define GUI_MARGIN              2

extern u16 gui_line, gui_column;
extern unsigned char iso_font[256 * 16];
extern const u8 la_logo[440], emex_logo[440];

int display_init(void);
void put_init(void);
void gui_putc(int c);
void gui_draw_logo(u64 scale, u64 x, u64 y, const u8 *logo);

#endif /* FW_DISPLAY_H */
