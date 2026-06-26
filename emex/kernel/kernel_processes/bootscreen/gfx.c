/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: gfx.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "gfx.h"

#include "boot.h"
#include "print.h"

#include "info.h"

#include <kernel/graph/graphics.h>
#include <kernel/include/reqs.h>
#include <kernel/include/assembly.h>

//from info.c
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

static u32 draw_logo(
    bs_screen_t *scr,
    u32 box_x,
    u32 box_y
){
    struct limine_file *logo_mod = find_logo_module();

    if (!logo_mod || logo_mod->size < LOGO_MIN_SIZE)
        return box_y + 20;

    const u8 *data = (const u8 *)logo_mod->address;

    u32 logo_w =
        (u32)data[0]            |
        ((u32)data[1] << 8)     |
        ((u32)data[2] << 16)    |
        ((u32)data[3] << 24);

    u32 logo_h =
        (u32)data[4]           |
        ((u32)data[5] << 8)    |
        ((u32)data[6] << 16)   |
        ((u32)data[7] << 24);

    u64 expected = 8 + ((u64)logo_w * logo_h * 4);

    if (logo_w == 0 || logo_h == 0 || logo_mod->size < expected) return box_y + 20;

    u32 lx = box_x + (600 - logo_w) / 2;
    u32 ly = box_y + 20;

    const u32 *pixels = (const u32 *)(data + 8);

    u32 draw_w = logo_w;
    u32 draw_h = logo_h;

    if (lx + draw_w > scr->width)
        draw_w = scr->width - lx;

    if (ly + draw_h > scr->height)
        draw_h = scr->height - ly;

    for (u32 dy = 0; dy < draw_h; dy++)
    {
        for (u32 dx = 0; dx < draw_w; dx++)
        {
            u32 c = pixels[dy * logo_w + dx];

            if ((c >> 24) == 0) continue;

            bs_setpixel(scr, lx + dx, ly + dy, c);
        }
    }

    return ly + draw_h + 8;
}

void loading_screen(void)
{
    bs_switch(BS4);

    log("::", "starting bootscreen gfx...\n", _d);

    int old = bs_active;
    bs_active = BS4;
    bs_screen_t *scr = &bs_screens[BS4];

    u32 len = scr->width * scr->height;
    if (scr->buffer) memset(scr->buffer, 0, len * sizeof(u32));

    //u32 text_y = 8;

    u32 w = bs_screens[BS4].width;
    u32 h = bs_screens[BS4].height;

    u32 box_w = 600;
    u32 box_h = 220;

    u32 box_x = (w - box_w) / 2;
    u32 box_y = (h - box_h) / 2;

    draw_rect(
        box_x + 5,
        box_y + 5,
        box_w,
        box_h,
        0xFFC0C0C0
    );

    draw_rect_both(
        box_x - 1,
        box_y - 1,
        box_w + 2,
        box_h + 2,
        0xFF000000,
        0xFF000000
    );

    draw_rect_both(
        box_x,
        box_y,
        box_w,
        box_h,
        0xFF000000,
        0xFFFFFFFF
    );

    u32 text_y = draw_logo(
        scr,
        box_x,
        box_y
    );

    scr->cursor_x = 4;
    scr->cursor_y = text_y;

    bs_screens[BS4].cursor_x = box_x + 20;
    bs_screens[BS4].cursor_y = text_y + 20;
    print_to(
        BS4,
        "| Loading " KERNEL_NAME"OS b" KERNEL_RELEASE_BUILD "("KERNEL_RELEASE_VER"), please wait...",
        0xFFFFFFFF
    );

    bs_backbuf_flush_all();
}