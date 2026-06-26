/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: qr.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "qr.h"
#include "qr_internal.h"

/*
 * soooo i need to implement qr-code version1 later cuz its very hard
 * and i just wanted to have some basics idk lol... so i would probably implemnt it in qr1.c
 */

int qr_generate(
    qr_t *qr,
    const char *text
)
{
    return qr_v1_generate(qr, text);
}

void qr_draw(
    const qr_t *qr,
    int x,
    int y,
    int scale,
    qr_draw_rect_fn draw_rect
)
{
    if (!qr || !draw_rect)
        return;

    for (int yy = 0; yy < qr->size; yy++)
    {
        for (int xx = 0; xx < qr->size; xx++)
        {
            if (!qr->modules[yy][xx])
                continue;

            draw_rect(
                x + xx * scale,
                y + yy * scale,
                scale,
                scale,
                0xFF000000
            );
        }
    }
}