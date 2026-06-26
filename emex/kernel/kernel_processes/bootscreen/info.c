/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: info.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "info.h"
#include "boot.h"
#include "config.h"
#include "print.h"

#include <kernel/include/reqs.h>
#include <kernel/graph/graphics.h>
#include <kernel/graph/theme.h>
#include <kernel/kernel_processes/fm/fm.h>
#include <kernel/cpu/cpu.h>
#include <kernel/graph/lib/string.h>
#include <config/system.h>
#include <types.h>

static struct limine_file *find_logo_module(void)
{
    if (!module_request.response) return NULL;
    if (module_request.response->module_count == 0) return NULL;

    struct limine_module_response *resp = module_request.response;

    for (u64 i = 0; i < resp->module_count; i++) {
        const char *path = resp->modules[i]->path;

        // extract filename from path
        const char *fname = path;
        for (const char *p = path; *p; p++) {
            if (*p == '/') fname = p + 1;
        }

        if (str_equals(fname, LOGO_MODULE_NAME)) {
            return resp->modules[i];
        }
    }
    return NULL;
}

void bs2_draw_info(void)
{
    int old = bs_active;
    bs_active = BS2;
    bs_screen_t *scr = &bs_screens[BS2];

    //print("bs2_draw_info", white());

    // clear BS2 local buffer
    u32 len = scr->width * scr->height;
    if (scr->buffer) memset(scr->buffer, 0, len * sizeof(u32));

    u32 text_y = 8;

    struct limine_file *logo_mod = find_logo_module();

    if (logo_mod && logo_mod->size >= LOGO_MIN_SIZE)
    {
        const u8 *data = (const u8 *)logo_mod->address;

        // read header
        u32 logo_w = (u32)data[0] | ((u32)data[1] << 8) |
                     ((u32)data[2] << 16) | ((u32)data[3] << 24);
        u32 logo_h = (u32)data[4] | ((u32)data[5] << 8) |
                     ((u32)data[6] << 16) | ((u32)data[7] << 24);

        u64 expected = 8 + (u64)logo_w * logo_h * 4;

        if (logo_w > 0 && logo_h > 0 && logo_mod->size >= expected)
        {
            const u32 *pixels = (const u32 *)(data + 8);

            // center horizontally in BS2
            //u32 lx = (scr->width > logo_w) ? (scr->width - logo_w) / 2 : 0;   // centered
            //u32 lx = (scr->width > logo_w) ? (scr->width - logo_w) / : 0; 	// right
            u32 lx = 16; // left; 16p padding
            u32 ly = 8;

            u32 draw_w = logo_w;
            u32 draw_h = logo_h;
            if (lx + draw_w > scr->width)  draw_w = scr->width  - lx;
            if (ly + draw_h > scr->height) draw_h = scr->height - ly;

            for (u32 dy = 0; dy < draw_h; dy++) {
                for (u32 dx = 0; dx < draw_w; dx++) {
                    u32 c = pixels[dy * logo_w + dx];
                    if ((c >> 24) == 0) continue; // transparent
                    bs_setpixel(scr, lx + dx, ly + dy, c);
                }
            }

            text_y = ly + draw_h + 8;
        }
    }

    // position text cursor below logo
    scr->cursor_x = 4;
    scr->cursor_y = text_y;

    information_text: {
    	f_setcontext(FONT_8X8_BOLD);
    	print_to(BS2, "\n" KERNEL_NAME " " KERNEL_RELEASE_BUILD "\n", cyan());
     	f_setcontext(FONT_8X8);

     	print_to(BS2,  ARCH_TEXT "" KERNEL_ARCH "\n", white());

        char inf[64];
        str_copy(inf, "Framebuffer:\n");
        str_append(inf, " \t Width:  ");
        str_append_uint(inf, fb_width);
        str_append(inf, "\n");
        str_append(inf, " \t Height: ");
        str_append_uint(inf, fb_height);
        str_append(inf, "\n");
        str_append(inf, " \t Pitch:  ");
        str_append_uint(inf, fb_pitch);
        str_append(inf, "\n");
        str_append(inf, " \t adress: ");
        str_append_uint(inf, (u64) framebuffer);
        str_append(inf, "\n");
        print_to(BS2, inf, GFX_WHITE);

        cpu_info_t *info = cpu_get_info();
        print_to(BS2, CPU_TEXT "" ,white());
        print_to(BS2, "\n", white());
        print_to(BS2, " - brand:   ", white());
        print_to(BS2, cpu_get_brand(), white());
        print_to(BS2, "\n", white());
        print_to(BS2, " - vendor:  ", white());
        print_to(BS2, cpu_get_vendor(), white());
        print_to(BS2, "\n", white());
        print_to(BS2, " - cores:   ", white());
        printInt_to(BS2, info->cores, white());
        print_to(BS2, "\n", white());
        print_to(BS2, " - threads: ", white());
        printInt_to(BS2, info->threads, white());
        print_to(BS2, "\n", white());
    }



    //print_to(BS2, "mode: limine\n", white());

    // flush BS2 to framebuffer
    {
        int saved = bs_active;
        bs_active = BS2;
        bs_flush_rect(0, 0, scr->width, scr->height);
        bs_active = saved;
    }

    bs_active = old;
}