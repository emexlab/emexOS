/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: vad.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

//#include "../emxfb0/emxfb0.h"

#define VAD_PALETTE_SIZE 256
#define VAD_MAGIC "TGRV" //54 47 52 56

typedef struct {
    int width;
    int height;

    unsigned char *indices;
    unsigned int  *pixels;

} vad_image_t;


const unsigned int *vad_get_palette(void);

void vad_set_palette(const unsigned int *palette);
void vad_set_color(int index, unsigned int argb);
void vad_reload_pixels(vad_image_t *img);
void vad_free(vad_image_t *img);

void vad_draw(const vad_image_t *img, int x, int y);
void vad_draw_scaled(const vad_image_t *img, int x, int y, int w, int h);

void vad_draw_ex(
    const vad_image_t *img,
    int x, int y, int w, int h,
    int sat, int bright, int alpha
);

void vad_set_bg(unsigned int color);
int vad_load(const char *path, vad_image_t *img);