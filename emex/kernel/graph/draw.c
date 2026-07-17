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

#include "graphics.h"
//#include "draw.h"
#include <kernel/kernel_processes/bootscreen/boot.h>


static inline void draw_pixel(u32 x, u32 y, u32 color)
{
	bs_screen_t *scr = bs_get_active();

	if (!scr) return;
	if (x >= scr->width || y >= scr->height) return;

	bs_setpixel(scr, x, y, color);
}

void draw_rect(u32 x, u32 y, u32 width, u32 height, u32 color)
{
	bs_screen_t *scr = bs_get_active();

	if (!scr || !scr->buffer) return;

    for (u32 dy = 0; dy < height; dy++)
    {
        for (u32 dx = 0; dx < width; dx++)
        {
        	draw_pixel(x + dx, y + dy, color);
        }
    }

    bs_flush_rect(x, y, width, height);
}

void draw_circle(u32 cx, u32 cy, u32 radius, u32 color)
{
	bs_screen_t *scr = bs_get_active();

	if (!scr || !scr->buffer) return;
    for (u32 y = 0; y <= radius; y++)
    {
        for (u32 x = 0; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                draw_pixel(cx + x, cy + y, color);
                draw_pixel(cx - x, cy + y, color);
                draw_pixel(cx + x, cy - y, color);
                draw_pixel(cx - x, cy - y, color);
            }
        }
    }

    bs_flush_rect(
    	cx - radius,
     	cy - radius,
     	radius * 2,
     	radius * 2
    );
}

void draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color)
{
	bs_screen_t *scr = bs_get_active();

	if (!scr || !scr->buffer) return;

	u32 ox0 = x0;
	u32 oy0 = y0;
	u32 ox1 = x1;
	u32 oy1 = y1;

    int dx = (int)x1 - (int)x0;
    int dy = (int)y1 - (int)y0;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        draw_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }

    u32 min_x = (ox0 < ox1) ? ox0 : ox1;
    u32 min_y = (oy0 < oy1) ? oy0 : oy1;
    u32 max_x = (ox0 < ox1) ? ox0 : ox1;
    u32 max_y = (oy0 < oy1) ? oy0 : oy1;

    bs_flush_rect(
     	min_x,
      	min_y,
       	(max_x - min_x) + 1,
        (max_y - min_y) + 1
    );
}

void draw_rect_outline(u32 x, u32 y, u32 width, u32 height, u32 color)
{
    draw_line(x, y, x + width - 1, y, color);
    draw_line(x, y, x, y + height - 1, color);

    draw_line(
        x + width - 1,
        y,
        x + width - 1,
        y + height - 1,
        color
    );

    draw_line(
        x,
        y + height - 1,
        x + width - 1,
        y + height - 1,
        color
    );
}

void draw_rect_both(
    u32 x,
    u32 y,
    u32 width,
    u32 height,
    u32 fill_color,
    u32 outline_color
)
{
    draw_rect(
        x,
        y,
        width,
        height,
        fill_color
    );

    draw_rect_outline(
        x,
        y,
        width,
        height,
        outline_color
    );
}
