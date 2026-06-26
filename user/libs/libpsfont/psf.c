/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: psf.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "psf.h"
#include "exui.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

extern eXui exui;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

int psf_load(const char *path, psf_font *font)
{
	FILE *f = fopen(path, "rb");

	if (!f) return 0;

	fread(&font->header, sizeof(psf1_header), 1, f);

	if (
		font->header.magic[0] !=  PSF1_MAGIC0   ||
		font->header.magic[1] !=  PSF1_MAGIC1
	) {
		fclose(f);
		return 0;
	}

	font->width = 8;
	font->height = font->header.charsize;
	font->glyph_count = (font->header.mode & 0x01) ? 512 : 256;
	int glyph_buffer_size = font->glyph_count * font->header.charsize;
	font->glyphs = malloc(glyph_buffer_size);

	if (!font->glyphs)
	{
		fclose(f);
		return 0;
	}

	fread(font->glyphs, glyph_buffer_size, 1, f);
	fclose(f);
	return 1;
}
void psf_free(psf_font *font)
{
	if(font->glyphs)
	{
		free(font->glyphs);
		font->glyphs = NULL;
	}
}
void psf_draw_char(
	psf_font *font,
	int x,
	int y,
	char c,
	unsigned int fg,
	unsigned int bg
) {
	uint8_t *glyph =font->glyphs + ((unsigned char)c * font->header.charsize);

	for (int row = 0; row < font->height; row++)
	{
		uint8_t bits = glyph[row];

		for(int col = 0; col < 8; col++)
		{
			unsigned int color = (bits & (0x80 >> col)) ? fg : bg;

			exui.DrawPixel(
				x + col,
				y + row,
				color
			);
		}
	}
}

void psf_print(
	psf_font *font,
	int x,
	int y,
	const char *text,
	unsigned int fg,
	unsigned int bg
){
	int cx = x;

	while (*text)
	{

		if (*text == '\n') {
			cx = x;
			y += font->height;
			text++;
			continue;
		}
		psf_draw_char(
			font,
			cx,
			y,
			*text,
			fg,
			bg
		);

		cx += font->width;

		text++;
	}
}