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

#include "fb0.h"

#include <kernel/graph/graphics.h>
#include <kernel/kernel_processes/bootscreen/boot.h>
#include <kernel/communication/serial.h>
#include <kernel/graph/lib/string.h>

#include <types.h>
#include <config/user.h>

#include <drivers/drivers.h>

static size_t fb_write_pos = 0;
static size_t fb_read_pos  = 0;

static int fb0_mod_init(void) {
    log("[FB0]", "init /dev/fb0\n", d);
    return 0;
}

static void fb0_mod_fini(void) {}

static void *fb0_open(const char *path)
{
    (void)path;
    fb_write_pos = 0;
    fb_read_pos = 0;
    return (void *)1;
}

static int fb0_read(void *handle, void *buf, size_t count, u64 offset)
{
    (void)handle;
    (void)offset;
    u32 *fb = bs_screens[USER_SCREEN_MODE].buffer;
    u32 pitch = bs_screens[USER_SCREEN_MODE].width * 4;
    u32 h = bs_screens[USER_SCREEN_MODE].height;

    if (!fb) return -1;

    size_t fb_size = (size_t)pitch * h;

    // EOF
    if (fb_read_pos >= fb_size) return 0;

    size_t remaining = fb_size - fb_read_pos;
    if (count > remaining) count = remaining;

    memcpy(buf, (u8 *)fb + fb_read_pos, count);
    fb_read_pos += count;
    return (int)count;
}

static int fb0_write(void *handle, const void *buf, size_t count, u64 offset)
{
    (void)handle;
    (void)offset;

    bs_screen_t *scr = &bs_screens[USER_SCREEN_MODE];
    u32 *fb_buf = scr->buffer;
    u32 pitch = scr->width * 4;
    u32 h = scr->height;

    if (!fb_buf) return -1;

    size_t fb_size = (size_t)pitch * h;

    // wrap around if we're on end
    if (fb_write_pos >= fb_size) fb_write_pos = 0;

    size_t remaining = fb_size - fb_write_pos;
    if (count > remaining) count = remaining;

    memcpy((u8 *)fb_buf + fb_write_pos, buf, count);

    u32 start_pixel = fb_write_pos / 4;
    u32 width = scr->width;
    u32 x = start_pixel % width;
    u32 y = start_pixel / width;
    u32 pixels_written = count / 4;
    u32 rect_w = (pixels_written > width) ? width : pixels_written;
    u32 rect_h = (pixels_written + width - 1) / width;

    fb_write_pos += count;

    /*needs to be the exact screen as userspace has,
     * the kernel DOES NOT use /dev/fb0, it should use the bs
     */
    //bs_switch(USER_SCREEN_MODE);
    //bs_flush_rect(x, y, rect_w, rect_h);
    bs_flush_rect_screen(USER_SCREEN_MODE, x, y, rect_w, rect_h);

    return (int)count;
}

int fb0_ioctl(int request, void *arg)
{
    if (!arg) return -1;

    u32 w = bs_screens[USER_SCREEN_MODE].width;
    u32 h = bs_screens[USER_SCREEN_MODE].height;
    u32 pitch = bs_screens[USER_SCREEN_MODE].width * 4;
    u32 *fb = bs_screens[USER_SCREEN_MODE].buffer;

    switch (request) {
        case FBIOGET_VSCREENINFO:
    	{
            fb_var_screeninfo_t *info = (fb_var_screeninfo_t *)arg;
            info->xres = w;
            info->yres = h;
            info->xres_virtual = w;
            info->yres_virtual = h;
            info->xoffset = 0;
            info->yoffset = 0;
            info->bits_per_pixel = 32;
            info->grayscale = 0;
            info->blue_offset = 0; info->blue_length  = 8;
            info->green_offset = 8; info->green_length = 8;
            info->red_offset = 16; info->red_length   = 8;
            info->transp_offset = 24; info->transp_length = 8;
            return 0;
        }
        case FBIOGET_FSCREENINFO:
    	{
            fb_fix_screeninfo_t *fix = (fb_fix_screeninfo_t *)arg;
            str_copy(fix->id, FBN);
            fix->smem_start = (u64)fb;
            fix->smem_len 	= pitch * h;
            fix->type   	= 0; // FB_TYPE_PACKED_PIXELS
            fix->visual 	= 2; // FB_VISUAL_TRUECOLOR
            fix->line_length = pitch;
            return 0;
        }
        case FBIO_READ_RECT:
    	{
            fb_rect_t *r = (fb_rect_t *)arg;
            if (!r || !r->pixels) return -1;

            // read from real fb instead of bs3
            u32 *real_fb = get_framebuffer();
            if (!real_fb) return -1;

            u32 real_pitch_dw = get_fb_pitch() / 4;

            for (u32 row = 0; row < r->h; row++)
            {
                u32 py = r->y + row;
                if (py >= h) break;
                for (u32 col = 0; col < r->w; col++)
                {
                    u32 px = r->x + col;
                    r->pixels[row * r->w + col] = (px < w) ? real_fb[py * real_pitch_dw + px] : 0;
                }
            }
            return 0;
        }
        case FBIO_BLIT:
    	{
            fb_rect_t *r = (fb_rect_t *)arg;
            if (!r || !r->pixels) return -1;

            u32 *real_fb   = get_framebuffer();
            u32 real_pitch_dw = get_fb_pitch() / 4;
            u32 pitch_dw   = pitch / 4;  // BS3 buffer pitch

            for (u32 row = 0; row < r->h; row++) {
                u32 py = r->y + row;
                if (py >= h) break;

                for (u32 col = 0; col < r->w; col++)
                {
                    u32 px = r->x + col;
                    if (px >= w) break;
                    u32 c = r->pixels[row * r->w + col];
                    if ((c >> 24) == 0) continue; // transparent
                    fb[py * pitch_dw + px] = c;

                    if (real_fb) real_fb[py * real_pitch_dw + px] = c;
                }
            }

            return 0;
        }
        case FBIO_RESET_POS:
            fb_write_pos = 0;
            return 0;
        default:
            return -1;
    }
}

driver_module fb0_module =
{
    .name    = FB0NAME,
    .mount   = FB0PATH,
    .version = FB0UNIVERSAL,
    .init    = fb0_mod_init,
    .fini    = fb0_mod_fini,
    .open    = fb0_open,
    .read    = fb0_read,
    .write   = fb0_write,
};
