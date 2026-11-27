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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nvram.h>

#define NVRAM_FILE_SIZE     0x10000
#define MAX_VARIABLES       256

struct var {
    char *name;
    char *value;
};

static struct var vars[MAX_VARIABLES];
static int var_count;

static int find(const char *name)
{
    for(int i = 0; i < var_count; i++)
    {
        if(strcmp(vars[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

static char *copy(const char *s,
                  size_t n)
{
    char *c = malloc(n + 1);
    if(c == NULL)
    {
        fprintf(stderr, "nvram: out of memory\n");
        exit(1);
    }
    memcpy(c, s, n);
    c[n] = 0;
    return c;
}

static void load(const uint8_t *data,
                 size_t size)
{
    size_t at = 0;
    while(at < size && data[at] != 0 && var_count < MAX_VARIABLES)
    {
        size_t end = at;
        while(end < size && data[end] != 0)
        {
            end++;
        }
        const char *eq = memchr(data + at, '=', end - at);
        if(eq != NULL)
        {
            size_t name_size = (size_t)(eq - (const char *)(data + at));
            vars[var_count].name = copy((const char *)data + at, name_size);
            vars[var_count].value = copy(eq + 1, end - at - name_size - 1);
            var_count++;
        }
        at = end + 1;
    }
}

static int common(uint8_t *nvram,
                  size_t size,
                  size_t *offset,
                  size_t *length)
{
    if(nvram_common(nvram, size, offset, length) == 0)
    {
        return 0;
    }

    size_t at = 0;
    while(size - at >= NVRAM_HEADER_SIZE)
    {
        const uint8_t *h = nvram + at;
        size_t blocks = ((size_t)h[2] << 8) | h[3];
        if(blocks == 0 || blocks * NVRAM_HEADER_SIZE > size - at || h[1] != nvram_checksum(h))
        {
            break;
        }
        at += blocks * NVRAM_HEADER_SIZE;
    }
    for(size_t i = at; i < size; i++)
    {
        if(nvram[i] != 0)
        {
            fprintf(stderr, "nvram: the NVRAM holds something else (UEFI variables?), it is left alone\n");
            return -1;
        }
    }

    size_t blocks = (size - at) / NVRAM_HEADER_SIZE;
    if(blocks > 0xFFFF)
    {
        blocks = 0xFFFF;
    }
    if(blocks < 2)
    {
        fprintf(stderr, "nvram: there is no room for the common partition\n");
        return -1;
    }

    uint8_t *h = nvram + at;
    h[0] = NVRAM_SIG_COMMON;
    h[2] = (uint8_t)(blocks >> 8);
    h[3] = (uint8_t)blocks;
    memcpy(h + 4, NVRAM_NAME_COMMON, sizeof(NVRAM_NAME_COMMON));
    h[1] = nvram_checksum(h);

    *offset = at + NVRAM_HEADER_SIZE;
    *length = blocks * NVRAM_HEADER_SIZE - NVRAM_HEADER_SIZE;
    return 0;
}

static int store(uint8_t *data,
                 size_t size)
{
    size_t at = 0;
    for(int i = 0; i < var_count; i++)
    {
        size_t name_size = strlen(vars[i].name);
        size_t value_size = strlen(vars[i].value);
        if(name_size + 1 + value_size + 1 + 1 > size - at)
        {
            fprintf(stderr, "nvram: the variables do not fit into the NVRAM\n");
            return -1;
        }
        memcpy(data + at, vars[i].name, name_size);
        data[at + name_size] = '=';
        memcpy(data + at + name_size + 1, vars[i].value, value_size);
        data[at + name_size + 1 + value_size] = 0;
        at += name_size + 1 + value_size + 1;
    }
    memset(data + at, 0, size - at);
    return 0;
}

static void usage(void)
{
    fprintf(stderr,
            "usage: nvram -f <file> -p\n"
            "       nvram -f <file> <name>\n"
            "       nvram -f <file> <name>=<value>\n"
            "       nvram -f <file> -d <name>\n"
            "       nvram -f <file> -c\n");
}

int main(int argc, char **argv)
{
    if(argc < 4 || strcmp(argv[1], "-f") != 0)
    {
        usage();
        return 2;
    }
    const char *path = argv[2];

    uint8_t *nvram;
    size_t size = NVRAM_FILE_SIZE;
    FILE *f = fopen(path, "rb");
    if(f != NULL)
    {
        if(fseek(f, 0, SEEK_END) != 0 || (size = (size_t)ftell(f)) == 0 || size > 16 * 1024 * 1024)
        {
            fprintf(stderr, "nvram: %s is no NVRAM file\n", path);
            fclose(f);
            return 1;
        }
        rewind(f);
        nvram = calloc(1, size);
        if(nvram == NULL || fread(nvram, 1, size, f) != size)
        {
            fprintf(stderr, "nvram: cannot read %s (%s)\n", path, strerror(errno));
            fclose(f);
            return 1;
        }
        fclose(f);
    }
    else if(errno == ENOENT)
    {
        nvram = calloc(1, size);
        if(nvram == NULL)
        {
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "nvram: cannot open %s (%s)\n", path, strerror(errno));
        return 1;
    }

    size_t offset, length;
    int have_common = nvram_common(nvram, size, &offset, &length) == 0;
    if(have_common)
    {
        load(nvram + offset, length);
    }

    int changed = 0;
    for(int a = 3; a < argc; a++)
    {
        const char *arg = argv[a];

        if(strcmp(arg, "-p") == 0)
        {
            for(int i = 0; i < var_count; i++)
            {
                printf("%s\t%s\n", vars[i].name, vars[i].value);
            }
        }
        else if(strcmp(arg, "-c") == 0)
        {
            var_count = 0;
            changed = 1;
        }
        else if(strcmp(arg, "-d") == 0)
        {
            if(++a == argc)
            {
                usage();
                return 2;
            }
            int i = find(argv[a]);
            if(i >= 0)
            {
                vars[i] = vars[--var_count];
                changed = 1;
            }
        }
        else if(strchr(arg, '=') != NULL)
        {
            const char *eq = strchr(arg, '=');
            if(eq == arg)
            {
                fprintf(stderr, "nvram: %s has no name\n", arg);
                return 1;
            }
            char *name = copy(arg, (size_t)(eq - arg));
            int i = find(name);
            if(i < 0)
            {
                if(var_count == MAX_VARIABLES)
                {
                    fprintf(stderr, "nvram: too many variables\n");
                    return 1;
                }
                i = var_count++;
                vars[i].name = name;
            }
            vars[i].value = copy(eq + 1, strlen(eq + 1));
            changed = 1;
        }
        else if(arg[0] == '-')
        {
            usage();
            return 2;
        }
        else
        {
            int i = find(arg);
            if(i < 0)
            {
                fprintf(stderr, "nvram: %s is not set\n", arg);
                return 1;
            }
            printf("%s\t%s\n", vars[i].name, vars[i].value);
        }
    }

    if(!changed)
    {
        return 0;
    }
    if(!have_common && common(nvram, size, &offset, &length) != 0)
    {
        return 1;
    }
    if(store(nvram + offset, length) != 0)
    {
        return 1;
    }

    f = fopen(path, "wb");
    if(f == NULL || fwrite(nvram, 1, size, f) != size || fclose(f) != 0)
    {
        fprintf(stderr, "nvram: cannot write %s (%s)\n", path, strerror(errno));
        return 1;
    }
    return 0;
}
