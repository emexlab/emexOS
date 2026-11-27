/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
 *
 * Private use and modification are permitted under the license.
 * Redistribution of source or binaries, modified or unmodified,
 * requires prior written permission from emexLabs.
 *
 * External deployment is subject to the source-availability
 * requirements and other conditions of the license.
 *
 * Provided "AS IS", without warranty, to the extent permitted by law.
 *
 * See LICENSE for the full license terms.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include <logo.h>

struct png {
    uint32_t width;
    uint32_t height;
    uint8_t depth;
    uint8_t type;
    uint8_t palette[256][4];
    int palette_size;
    int has_key;
    uint16_t key[3];
};

static uint32_t be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static int fail(const char *why)
{
    fprintf(stderr, "logo: %s\n", why);
    return 1;
}

static uint8_t *read_file(const char *path,
                          size_t *size)
{
    FILE *f = fopen(path, "rb");
    if(f == NULL)
    {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);
    uint8_t *data = n > 0 ? malloc((size_t)n) : NULL;
    if(data == NULL || fread(data, 1, (size_t)n, f) != (size_t)n)
    {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *size = (size_t)n;
    return data;
}

static int paeth(int a,
                 int b,
                 int c)
{
    int p = a + b - c;
    int pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
    return pa <= pb && pa <= pc ? a : pb <= pc ? b : c;
}

static uint8_t *decode(const uint8_t *file,
                       size_t size,
                       struct png *png)
{
    static const uint8_t signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
    uint8_t *idat = NULL;
    size_t idat_size = 0;
    int channels = 0;

    if(size < 8 || memcmp(file, signature, 8) != 0)
    {
        fail("not a PNG file");
        return NULL;
    }

    memset(png, 0, sizeof(*png));
    for(size_t at = 8; at + 12 <= size;)
    {
        uint32_t length = be32(file + at);
        const uint8_t *type = file + at + 4;
        const uint8_t *data = file + at + 8;
        if(length > size - at - 12)
        {
            fail("the PNG file is cut short");
            return NULL;
        }

        if(memcmp(type, "IHDR", 4) == 0 && length == 13)
        {
            png->width = be32(data);
            png->height = be32(data + 4);
            png->depth = data[8];
            png->type = data[9];
            if(data[10] != 0 || data[11] != 0)
            {
                fail("unknown PNG compression or filter");
                return NULL;
            }
            if(data[12] != 0)
            {
                fail("interlaced PNG files are not read, save it without interlacing");
                return NULL;
            }
        }
        else if(memcmp(type, "PLTE", 4) == 0 && length % 3 == 0 && length <= 768)
        {
            png->palette_size = (int)(length / 3);
            for(int i = 0; i < png->palette_size; i++)
            {
                png->palette[i][0] = data[i * 3];
                png->palette[i][1] = data[i * 3 + 1];
                png->palette[i][2] = data[i * 3 + 2];
                png->palette[i][3] = 255;
            }
        }
        else if(memcmp(type, "tRNS", 4) == 0)
        {
            if(png->type == 3)
            {
                for(uint32_t i = 0; i < length && i < 256; i++)
                {
                    png->palette[i][3] = data[i];
                }
            }
            else if(png->type == 0 && length == 2)
            {
                png->has_key = 1;
                png->key[0] = png->key[1] = png->key[2] = (uint16_t)((data[0] << 8) | data[1]);
            }
            else if(png->type == 2 && length == 6)
            {
                png->has_key = 1;
                for(int c = 0; c < 3; c++)
                {
                    png->key[c] = (uint16_t)((data[c * 2] << 8) | data[c * 2 + 1]);
                }
            }
        }
        else if(memcmp(type, "IDAT", 4) == 0)
        {
            uint8_t *more = realloc(idat, idat_size + length);
            if(more == NULL)
            {
                free(idat);
                fail("out of memory");
                return NULL;
            }
            idat = more;
            memcpy(idat + idat_size, data, length);
            idat_size += length;
        }
        else if(memcmp(type, "IEND", 4) == 0)
        {
            break;
        }
        at += 12 + length;
    }

    switch(png->type)
    {
        case 0: channels = 1; break;
        case 2: channels = 3; break;
        case 3: channels = 1; break;
        case 4: channels = 2; break;
        case 6: channels = 4; break;
        default:
            free(idat);
            fail("unknown PNG color type");
            return NULL;
    }
    int depth = png->depth;
    if(!(depth == 8 || depth == 16 || ((png->type == 0 || png->type == 3) && (depth == 1 || depth == 2 || depth == 4))) ||
       (png->type == 3 && (depth == 16 || png->palette_size == 0)))
    {
        free(idat);
        fail("unsupported PNG bit depth");
        return NULL;
    }
    if(png->width == 0 || png->height == 0 || png->width > LOGO_MAX_SIDE || png->height > LOGO_MAX_SIDE || idat == NULL)
    {
        free(idat);
        fail("the PNG picture is empty or larger than 4096 pixels on a side");
        return NULL;
    }

    size_t bits = (size_t)png->width * (size_t)channels * (size_t)depth;
    size_t stride = (bits + 7) / 8;
    size_t raw_size = (stride + 1) * png->height;
    uint8_t *raw = malloc(raw_size);
    uLongf out_size = (uLongf)raw_size;
    if(raw == NULL || uncompress(raw, &out_size, idat, (uLong)idat_size) != Z_OK || out_size != raw_size)
    {
        free(idat);
        free(raw);
        fail("the PNG picture data is damaged");
        return NULL;
    }
    free(idat);

    size_t bpp = ((size_t)channels * (size_t)depth + 7) / 8;
    for(uint32_t y = 0; y < png->height; y++)
    {
        uint8_t *row = raw + y * (stride + 1);
        uint8_t filter = row[0];
        uint8_t *line = row + 1;
        uint8_t *prev = y > 0 ? raw + (y - 1) * (stride + 1) + 1 : NULL;
        for(size_t x = 0; x < stride; x++)
        {
            int a = x >= bpp ? line[x - bpp] : 0;
            int b = prev != NULL ? prev[x] : 0;
            int c = prev != NULL && x >= bpp ? prev[x - bpp] : 0;
            switch(filter)
            {
                case 0: break;
                case 1: line[x] = (uint8_t)(line[x] + a); break;
                case 2: line[x] = (uint8_t)(line[x] + b); break;
                case 3: line[x] = (uint8_t)(line[x] + ((a + b) >> 1)); break;
                case 4: line[x] = (uint8_t)(line[x] + paeth(a, b, c)); break;
                default:
                    free(raw);
                    fail("the PNG picture data is damaged");
                    return NULL;
            }
        }
    }

    uint8_t *rgba = malloc((size_t)png->width * png->height * 4);
    if(rgba == NULL)
    {
        free(raw);
        fail("out of memory");
        return NULL;
    }
    for(uint32_t y = 0; y < png->height; y++)
    {
        const uint8_t *line = raw + y * (stride + 1) + 1;
        for(uint32_t x = 0; x < png->width; x++)
        {
            uint8_t *out = rgba + ((size_t)y * png->width + x) * 4;
            uint16_t s[4];
            for(int c = 0; c < channels; c++)
            {
                if(depth < 8)
                {
                    size_t bit = ((size_t)x * channels + c) * depth;
                    s[c] = (uint16_t)((line[bit / 8] >> (8 - depth - bit % 8)) & ((1 << depth) - 1));
                }
                else if(depth == 8)
                {
                    s[c] = line[(size_t)x * channels + c];
                }
                else
                {
                    const uint8_t *p = line + ((size_t)x * channels + c) * 2;
                    s[c] = (uint16_t)((p[0] << 8) | p[1]);
                }
            }

            int max = (1 << depth) - 1;
            #define TO8(v) ((uint8_t)(depth == 16 ? (v) >> 8 : depth == 8 ? (v) : (v) * 255 / max))
            switch(png->type)
            {
                case 0:
                    out[0] = out[1] = out[2] = TO8(s[0]);
                    out[3] = png->has_key && s[0] == png->key[0] ? 0 : 255;
                    break;
                case 2:
                    out[0] = TO8(s[0]);
                    out[1] = TO8(s[1]);
                    out[2] = TO8(s[2]);
                    out[3] = png->has_key && s[0] == png->key[0] && s[1] == png->key[1] && s[2] == png->key[2] ? 0 : 255;
                    break;
                case 3:
                    if(s[0] >= png->palette_size)
                    {
                        free(raw);
                        free(rgba);
                        fail("the PNG picture uses a color its palette does not have");
                        return NULL;
                    }
                    memcpy(out, png->palette[s[0]], 4);
                    break;
                case 4:
                    out[0] = out[1] = out[2] = TO8(s[0]);
                    out[3] = TO8(s[1]);
                    break;
                case 6:
                    out[0] = TO8(s[0]);
                    out[1] = TO8(s[1]);
                    out[2] = TO8(s[2]);
                    out[3] = TO8(s[3]);
                    break;
            }
            #undef TO8
        }
    }
    free(raw);
    return rgba;
}

static void le32(uint8_t *p,
                 uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

int main(int argc, char **argv)
{
    if(argc != 3 && argc != 4)
    {
        fprintf(stderr, "usage: logo <in.png> <out.logo> [size]\n");
        return 2;
    }
    long max_side = argc == 4 ? strtol(argv[3], NULL, 10) : 128;
    if(max_side < 1 || max_side > LOGO_MAX_SIDE)
    {
        fprintf(stderr, "logo: the size is 1 to %d pixels\n", LOGO_MAX_SIDE);
        return 2;
    }

    size_t size;
    uint8_t *file = read_file(argv[1], &size);
    if(file == NULL)
    {
        fprintf(stderr, "logo: cannot read %s\n", argv[1]);
        return 1;
    }
    struct png png;
    uint8_t *rgba = decode(file, size, &png);
    free(file);
    if(rgba == NULL)
    {
        return 1;
    }

    size_t pixels = (size_t)png.width * png.height;
    for(size_t i = 0; i < pixels; i++)
    {
        uint8_t *p = rgba + i * 4;
        for(int c = 0; c < 3; c++)
        {
            p[c] = (uint8_t)(p[c] * p[3] / 255);
        }
    }

    if(png.width > (uint32_t)max_side || png.height > (uint32_t)max_side)
    {
        uint32_t w, h;
        if(png.width >= png.height)
        {
            w = (uint32_t)max_side;
            h = (uint32_t)(((uint64_t)png.height * w + png.width / 2) / png.width);
        }
        else
        {
            h = (uint32_t)max_side;
            w = (uint32_t)(((uint64_t)png.width * h + png.height / 2) / png.height);
        }
        w = w ? w : 1;
        h = h ? h : 1;

        uint8_t *small = malloc((size_t)w * h * 4);
        for(uint32_t y = 0; y < h; y++)
        {
            uint32_t y0 = (uint32_t)((uint64_t)y * png.height / h);
            uint32_t y1 = (uint32_t)((uint64_t)(y + 1) * png.height / h);
            y1 = y1 > y0 ? y1 : y0 + 1;
            for(uint32_t x = 0; x < w; x++)
            {
                uint32_t x0 = (uint32_t)((uint64_t)x * png.width / w);
                uint32_t x1 = (uint32_t)((uint64_t)(x + 1) * png.width / w);
                x1 = x1 > x0 ? x1 : x0 + 1;
                uint64_t sum[4] = { 0 };
                for(uint32_t sy = y0; sy < y1; sy++)
                {
                    for(uint32_t sx = x0; sx < x1; sx++)
                    {
                        const uint8_t *p = rgba + ((size_t)sy * png.width + sx) * 4;
                        for(int c = 0; c < 4; c++)
                        {
                            sum[c] += p[c];
                        }
                    }
                }
                uint64_t n = (uint64_t)(y1 - y0) * (x1 - x0);
                for(int c = 0; c < 4; c++)
                {
                    small[((size_t)y * w + x) * 4 + c] = (uint8_t)((sum[c] + n / 2) / n);
                }
            }
        }
        free(rgba);
        rgba = small;
        png.width = w;
        png.height = h;
        pixels = (size_t)w * h;
    }

    for(size_t i = 0; i < pixels; i++)
    {
        uint8_t *p = rgba + i * 4;
        if(p[3] < 16 || (p[0] < 8 && p[1] < 8 && p[2] < 8))
        {
            p[3] = 0;
            continue;
        }
        p[3] = 255;
    }

    uint8_t *index = calloc(1, pixels);
    uint32_t colors[LOGO_MAX_COLORS];
    int count = 0, cube = 0;

    for(size_t i = 0; i < pixels && !cube; i++)
    {
        const uint8_t *p = rgba + i * 4;
        if(p[3] == 0)
        {
            continue;
        }
        uint32_t rgb = ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
        int k;
        for(k = 0; k < count && colors[k] != rgb; k++)
        {
        }
        if(k == count)
        {
            if(count == LOGO_MAX_COLORS)
            {
                cube = 1;
                break;
            }
            colors[count++] = rgb;
        }
        index[i] = (uint8_t)(k + 1);
    }

    if(cube)
    {
        count = 6 * 7 * 6;
        for(int r = 0; r < 6; r++)
        {
            for(int g = 0; g < 7; g++)
            {
                for(int b = 0; b < 6; b++)
                {
                    colors[(r * 7 + g) * 6 + b] = ((uint32_t)(r * 255 / 5) << 16) | ((uint32_t)(g * 255 / 6) << 8) | (uint32_t)(b * 255 / 5);
                }
            }
        }
        for(size_t i = 0; i < pixels; i++)
        {
            const uint8_t *p = rgba + i * 4;
            if(p[3] == 0)
            {
                index[i] = 0;
                continue;
            }
            int r = (p[0] * 5 + 127) / 255, g = (p[1] * 6 + 127) / 255, b = (p[2] * 5 + 127) / 255;
            index[i] = (uint8_t)((r * 7 + g) * 6 + b + 1);
        }
        fprintf(stderr, "logo: %s has more than %d colors, they are reduced to %d\n", argv[1], LOGO_MAX_COLORS, count);
    }
    if(count == 0)
    {
        free(rgba);
        free(index);
        return fail("the picture is transparent all over");
    }

    size_t out_size = LOGO_HEADER_SIZE + (size_t)count * 3 + pixels;
    uint8_t *out = calloc(1, out_size);
    memcpy(out, LOGO_MAGIC, 8);
    le32(out + 8, png.width);
    le32(out + 12, png.height);
    le32(out + 16, (uint32_t)count);
    for(int k = 0; k < count; k++)
    {
        out[LOGO_HEADER_SIZE + k * 3] = (uint8_t)(colors[k] >> 16);
        out[LOGO_HEADER_SIZE + k * 3 + 1] = (uint8_t)(colors[k] >> 8);
        out[LOGO_HEADER_SIZE + k * 3 + 2] = (uint8_t)colors[k];
    }
    memcpy(out + LOGO_HEADER_SIZE + (size_t)count * 3, index, pixels);

    FILE *f = fopen(argv[2], "wb");
    if(f == NULL || fwrite(out, 1, out_size, f) != out_size || fclose(f) != 0)
    {
        fprintf(stderr, "logo: cannot write %s\n", argv[2]);
        return 1;
    }
    printf("logo: %ux%u, %d colors\n", png.width, png.height, count);
    free(rgba);
    free(index);
    free(out);
    return 0;
}
