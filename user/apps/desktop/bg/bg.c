/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: bg.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "bg.h"
#include "../compositor/comp.h"
#include "../config/cfg.h"
#include "../../../libs/libbmp/bmp.h"
#include <stdio.h>
//#include <stdlib.h>

static bmp_image_t g_bg;
static int g_bg_loaded = 0;

#define BG_ROW_MAX 4096
static unsigned int g_row_buf[BG_ROW_MAX];

void bg_init(int w, int h)
{
    (void)w;
    (void)h;

    const char *path = BG_PATH;

    printf(":: bg: trying to load bg picture from path: %s\n", path);

    int res = bmp_load(path, &g_bg);

    printf("[BG] bmp_load result: %d\n", res);

    if (res != 0) {
        printf("failed to unknown reason\n");
        g_bg_loaded = 0;
        return;
    }
    if (!g_bg.pixels) {
        printf("pixels pointer is NULL\n");
        g_bg_loaded = 0;
        return;
    }
    if (g_bg.width <= 0 || g_bg.height <= 0) {
        printf("return. %dx%d\n", g_bg.width, g_bg.height);
        g_bg_loaded = 0;
        return;
    }

    printf("bg: resolution: %dx%d\n", g_bg.width, g_bg.height);
    printf("bg: pixels ptr: %p\n", g_bg.pixels);


    int sw = comp_w();
    int sh = comp_h();

    printf(":: analysing screen... %dx%d\n", sw, sh);

    if (g_bg.width > sw || g_bg.height > sh) {
        printf("warning: bg pic. too large...\n");
    }
    if (g_bg.width < sw || g_bg.height < sh) {
        printf("warning: bg pic. is smaller than the screen...\n");
    }

    unsigned char *p = (unsigned char *)g_bg.pixels;
    printf("bg: %02X %02X %02X %02X (if available)\n", p[0], p[1], p[2], p[3]);

    g_bg_loaded = 1;

    //printf("done\n");
}

void bg_draw_full(void)
{
    int sw = comp_w();
    int sh = comp_h();
    int x;
    int y;

    if (!g_bg_loaded || !g_bg.pixels) {
        comp_fill(0, 0, sw, sh, DT_BG);
        return;
    }

    int bw = g_bg.width;
    int bh = g_bg.height;
    int rlen = sw < BG_ROW_MAX ? sw : BG_ROW_MAX;

    for (y = 0; y < sh; y++)
    {
        int sy = (y * bh) / sh;

        for (x = 0; x < rlen; x++)
        {
            int sx = (x * bw) / sw;
            g_row_buf[x] = g_bg.pixels[sy * bw + sx];
        }

        comp_put_pixels(0, y, rlen, 1, g_row_buf);
    }

    //free(row);
}

void bg_draw_rect(int x, int y, int w, int h)
{
    int sw = comp_w();
    int sh = comp_h();
    int dy, dx;

    if (!g_bg_loaded || !g_bg.pixels) {
        comp_fill(x, y, w, h, DT_BG);
        return;
    }

    int bw = g_bg.width;
    int bh = g_bg.height;
    int rlen = w < BG_ROW_MAX ? w : BG_ROW_MAX;

    for (dy = 0; dy < h; dy++)
    {
        int sy = ((y + dy) * bh) / sh;

        for (dx = 0; dx < rlen; dx++)
        {
            int sx = ((x + dx) * bw) / sw;
            g_row_buf[dx] = g_bg.pixels[sy * bw + sx];
        }

        comp_put_pixels(x, y + dy, rlen, 1, g_row_buf);
    }

    //free(row);
}
