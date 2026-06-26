/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: psf.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include <stdint.h>

typedef struct
{
	uint8_t magic[2];
	uint8_t mode;
	uint8_t charsize;
} psf1_header;
typedef struct
{
	psf1_header header;

	uint8_t *glyphs;
	int width;
	int height;
	int glyph_count;
} psf_font;

int psf_load(const char *path, psf_font *font);
void psf_free(psf_font *font);
void psf_draw_char(
	psf_font *font,
	int x,
	int y,
	char c,
	unsigned int fg,
	unsigned int bg
);
void psf_print(
	psf_font *font,
	int x,
	int y,
	const char *text,
	unsigned int fg,
	unsigned int bg
);