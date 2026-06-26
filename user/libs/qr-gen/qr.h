/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: qr.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#define QR_MAX_SIZE 21

typedef struct
{
    int size;
    unsigned char modules[QR_MAX_SIZE][QR_MAX_SIZE];
} qr_t;

typedef void (*qr_draw_rect_fn)(
    int x,
    int y,
    int w,
    int h,
    unsigned int color
);

int qr_generate(
    qr_t *qr,
    const char *text
);

void qr_draw(
    const qr_t *qr,
    int x,
    int y,
    int scale,
    qr_draw_rect_fn draw_rect
);