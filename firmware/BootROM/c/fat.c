/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.1
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.1
 * (EPSL 1.1).
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

#include <stddef.h>
#include <stdint.h>
#include <fat.h>

int fat_mount(struct fat_volume *volume, fat_read_fn read, void *context);
int fat_mount_at(struct fat_volume *volume, fat_read_fn read, void *context, uint64_t start);
int fat_size(struct fat_volume *volume, const char *path, uint64_t *size);
int fat_load(struct fat_volume *volume, const char *path, void *buffer, uint64_t room, uint64_t *size);

#define FAT_SECTOR          512
#define FAT_MAX_SECTOR      4096

static uint8_t fat_cache[FAT_MAX_SECTOR];
static const struct fat_volume *fat_cache_volume;
static uint64_t fat_cache_sector = ~(uint64_t)0;

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t rd64(const uint8_t *p)
{
    return (uint64_t)rd32(p) | ((uint64_t)rd32(p + 4) << 32);
}

static void copy(void *dst,
                 const void *src,
                 size_t n)
{
    uint8_t *d = dst;
    const uint8_t *s = src;
    while(n--)
    {
        *d++ = *s++;
    }
}

static int lower(int c)
{
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

static int fs_read(const struct fat_volume *v,
                   uint64_t sector,
                   uint32_t count,
                   void *buffer)
{
    if(v->read(v->context, v->start + sector * v->scale, count * v->scale, buffer) != 0)
    {
        return FAT_ERR_IO;
    }
    return FAT_OK;
}

static const uint8_t *fs_sector(const struct fat_volume *v,
                                uint64_t sector)
{
    if(fat_cache_volume != v || fat_cache_sector != sector)
    {
        fat_cache_volume = NULL;
        if(fs_read(v, sector, 1, fat_cache) != FAT_OK)
        {
            return NULL;
        }
        fat_cache_volume = v;
        fat_cache_sector = sector;
    }
    return fat_cache;
}

static uint32_t fat_next(const struct fat_volume *v,
                         uint32_t cluster)
{
    uint64_t offset;
    uint32_t next;
    const uint8_t *s;

    if(cluster < 2 || cluster >= v->clusters + 2)
    {
        return 0;
    }

    switch(v->type)
    {
        case 12:
        {
            uint8_t pair[2];
            offset = cluster + cluster / 2;
            s = fs_sector(v, v->fat_start + offset / v->sector_size);
            if(s == NULL)
            {
                return 0;
            }
            pair[0] = s[offset % v->sector_size];
            if((offset + 1) % v->sector_size == 0)
            {
                s = fs_sector(v, v->fat_start + (offset + 1) / v->sector_size);
                if(s == NULL)
                {
                    return 0;
                }
                pair[1] = s[0];
            }
            else
            {
                pair[1] = s[(offset + 1) % v->sector_size];
            }
            next = rd16(pair);
            next = (cluster & 1) ? next >> 4 : next & 0xFFF;
            return next >= 0xFF8 ? 0 : next;
        }
        case 16:
            offset = (uint64_t)cluster * 2;
            s = fs_sector(v, v->fat_start + offset / v->sector_size);
            if(s == NULL)
            {
                return 0;
            }
            next = rd16(s + offset % v->sector_size);
            return next >= 0xFFF8 ? 0 : next;
        default:
            offset = (uint64_t)cluster * 4;
            s = fs_sector(v, v->fat_start + offset / v->sector_size);
            if(s == NULL)
            {
                return 0;
            }
            next = rd32(s + offset % v->sector_size) & 0x0FFFFFFF;
            return next >= 0x0FFFFFF8 ? 0 : next;
    }
}

static uint64_t cluster_sector(const struct fat_volume *v,
                               uint32_t cluster)
{
    return v->data_start + (uint64_t)(cluster - 2) * v->cluster_sectors;
}

int fat_mount_at(struct fat_volume *volume,
                 fat_read_fn read,
                 void *context,
                 uint64_t start)
{
    static uint8_t boot[FAT_MAX_SECTOR];
    struct fat_volume v;
    uint32_t reserved, fats, root_entries, total, fat_size, data;

    v.read = read;
    v.context = context;
    v.start = start;
    if(read(context, start, 1, boot) != 0)
    {
        return FAT_ERR_IO;
    }

    v.sector_size = rd16(boot + 11);
    v.cluster_sectors = boot[13];
    reserved = rd16(boot + 14);
    fats = boot[16];
    root_entries = rd16(boot + 17);
    total = rd16(boot + 19) ? rd16(boot + 19) : rd32(boot + 32);
    fat_size = rd16(boot + 22) ? rd16(boot + 22) : rd32(boot + 36);

    if(rd16(boot + 510) != 0xAA55 ||
       (v.sector_size != 512 && v.sector_size != 1024 && v.sector_size != 2048 && v.sector_size != 4096) ||
       v.cluster_sectors == 0 || (v.cluster_sectors & (v.cluster_sectors - 1)) != 0 ||
       reserved == 0 || fats == 0 || fat_size == 0 || total == 0)
    {
        return FAT_ERR_NO_VOLUME;
    }

    v.scale = v.sector_size / FAT_SECTOR;
    if(v.scale > 1 && read(context, start + 1, v.scale - 1, boot + FAT_SECTOR) != 0)
    {
        return FAT_ERR_IO;
    }

    v.cluster_size = v.sector_size * v.cluster_sectors;
    v.fat_start = reserved;
    v.root_start = reserved + fats * fat_size;
    v.root_sectors = (root_entries * 32 + v.sector_size - 1) / v.sector_size;
    v.data_start = v.root_start + v.root_sectors;
    if(v.data_start >= total)
    {
        return FAT_ERR_NO_VOLUME;
    }
    data = total - v.data_start;
    v.clusters = data / v.cluster_sectors;

    if(v.clusters < 4085)
    {
        v.type = 12;
    }
    else if(v.clusters < 65525)
    {
        v.type = 16;
    }
    else
    {
        v.type = 32;
    }
    v.root_cluster = v.type == 32 ? rd32(boot + 44) : 0;
    if(v.type == 32 && (root_entries != 0 || v.root_cluster < 2))
    {
        return FAT_ERR_NO_VOLUME;
    }

    *volume = v;
    if(fat_cache_volume == volume)
    {
        fat_cache_volume = NULL;
    }
    return FAT_OK;
}

static int guid_is_esp(const uint8_t *g)
{
    static const uint8_t esp[16] = {
        0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11,
        0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B,
    };
    for(int i = 0; i < 16; i++)
    {
        if(g[i] != esp[i])
        {
            return 0;
        }
    }
    return 1;
}

static uint64_t gpt_esp(fat_read_fn read, void *context)
{
    static uint8_t sector[FAT_SECTOR];
    uint64_t entries_lba;
    uint32_t count, entry_size;

    if(read(context, 1, 1, sector) != 0)
    {
        return 0;
    }
    if(sector[0] != 'E' || sector[1] != 'F' || sector[2] != 'I' || sector[3] != ' ' ||
       sector[4] != 'P' || sector[5] != 'A' || sector[6] != 'R' || sector[7] != 'T')
    {
        return 0;
    }
    entries_lba = rd64(sector + 72);
    count = rd32(sector + 80);
    entry_size = rd32(sector + 84);
    if(entry_size < 128 || entry_size > FAT_SECTOR || (FAT_SECTOR % entry_size) != 0 || count > 1024)
    {
        return 0;
    }

    for(uint32_t i = 0; i < count; i++)
    {
        uint32_t at = (i * entry_size) % FAT_SECTOR;
        if(at == 0 && read(context, entries_lba + (uint64_t)i * entry_size / FAT_SECTOR, 1, sector) != 0)
        {
            return 0;
        }
        if(guid_is_esp(sector + at))
        {
            return rd64(sector + at + 32);
        }
    }
    return 0;
}

int fat_mount(struct fat_volume *volume,
              fat_read_fn read,
              void *context)
{
    static uint8_t mbr[FAT_SECTOR];
    uint64_t esp;
    int first = -1;

    esp = gpt_esp(read, context);
    if(esp != 0)
    {
        return fat_mount_at(volume, read, context, esp);
    }

    if(read(context, 0, 1, mbr) != 0)
    {
        return FAT_ERR_IO;
    }
    if(rd16(mbr + 510) == 0xAA55)
    {
        for(int i = 0; i < 4; i++)
        {
            const uint8_t *p = mbr + 446 + i * 16;
            uint8_t type = p[4];
            uint32_t lba = rd32(p + 8);
            if(lba == 0 || type == 0xEE)
            {
                continue;
            }
            if(type == 0xEF)
            {
                return fat_mount_at(volume, read, context, lba);
            }
            if(first < 0 && (type == 0x01 || type == 0x04 || type == 0x06 || type == 0x0B || type == 0x0C || type == 0x0E))
            {
                first = i;
            }
        }
        if(first >= 0)
        {
            return fat_mount_at(volume, read, context, rd32(mbr + 446 + first * 16 + 8));
        }
    }

    return fat_mount_at(volume, read, context, 0);
}

struct fat_entry {
    uint32_t cluster;
    uint64_t size;
    int directory;
};

struct fat_cursor {
    const struct fat_volume *v;
    uint32_t cluster;
    uint64_t sector;
    uint32_t index;
};

static uint8_t fat_dir[FAT_MAX_SECTOR];

static const uint8_t *cursor_next(struct fat_cursor *c,
                                  int *error)
{
    const struct fat_volume *v = c->v;
    uint32_t per_sector = v->sector_size / 32;

    if(c->index == per_sector)
    {
        c->index = 0;
        c->sector++;
        if(c->cluster == 0)
        {
            if(c->sector >= v->root_sectors)
            {
                return NULL;
            }
        }
        else if(c->sector == v->cluster_sectors)
        {
            c->cluster = fat_next(v, c->cluster);
            c->sector = 0;
            if(c->cluster == 0)
            {
                return NULL;
            }
        }
    }

    if(c->index == 0)
    {
        uint64_t sector = c->cluster == 0 ? v->root_start + c->sector : cluster_sector(v, c->cluster) + c->sector;
        if(fs_read(v, sector, 1, fat_dir) != FAT_OK)
        {
            *error = FAT_ERR_IO;
            return NULL;
        }
    }
    return fat_dir + 32 * c->index++;
}

static int short_name_is(const uint8_t *entry,
                         const char *name,
                         size_t length)
{
    char text[13];
    size_t n = 0;

    for(int i = 0; i < 8 && entry[i] != ' '; i++)
    {
        text[n++] = (char)(i == 0 && entry[0] == 0x05 ? 0xE5 : entry[i]);
    }
    if(entry[8] != ' ')
    {
        text[n++] = '.';
        for(int i = 8; i < 11 && entry[i] != ' '; i++)
        {
            text[n++] = (char)entry[i];
        }
    }
    if(n != length)
    {
        return 0;
    }
    for(size_t i = 0; i < n; i++)
    {
        if(lower((uint8_t)text[i]) != lower((uint8_t)name[i]))
        {
            return 0;
        }
    }
    return 1;
}

static uint8_t lfn_checksum(const uint8_t *entry)
{
    uint8_t sum = 0;
    for(int i = 0; i < 11; i++)
    {
        sum = (uint8_t)(((sum & 1) << 7) + (sum >> 1) + entry[i]);
    }
    return sum;
}

static int dir_find(const struct fat_volume *v,
                    uint32_t cluster,
                    const char *name,
                    size_t length,
                    struct fat_entry *found)
{
    struct fat_cursor c = { .v = v, .cluster = cluster, .sector = 0, .index = 0 };
    static const uint8_t lfn_places[13] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
    uint16_t lfn[260];
    size_t lfn_length = 0;
    uint8_t lfn_sum = 0;
    int lfn_valid = 0;
    int error = FAT_OK;
    const uint8_t *e;

    while((e = cursor_next(&c, &error)) != NULL)
    {
        if(e[0] == 0x00)
        {
            break;
        }
        if(e[0] == 0xE5)
        {
            lfn_valid = 0;
            continue;
        }

        if(e[11] == 0x0F)
        {
            uint32_t part = e[0] & 0x1F;
            if(part == 0 || part > 20)
            {
                lfn_valid = 0;
                continue;
            }
            if(e[0] & 0x40)
            {
                lfn_valid = 1;
                lfn_sum = e[13];
                lfn_length = part * 13;
                for(size_t i = 0; i < lfn_length; i++)
                {
                    lfn[i] = 0xFFFF;
                }
            }
            else if(!lfn_valid || e[13] != lfn_sum || part * 13 > lfn_length)
            {
                lfn_valid = 0;
                continue;
            }
            for(int i = 0; i < 13; i++)
            {
                lfn[(part - 1) * 13 + i] = rd16(e + lfn_places[i]);
            }
            continue;
        }

        if(!(e[11] & 0x08))
        {
            int match = 0;
            if(lfn_valid && lfn_checksum(e) == lfn_sum)
            {
                size_t n = 0;
                while(n < lfn_length && lfn[n] != 0x0000 && lfn[n] != 0xFFFF)
                {
                    n++;
                }
                if(n == length)
                {
                    match = 1;
                    for(size_t i = 0; i < n; i++)
                    {
                        if(lfn[i] > 0x7F || lower(lfn[i]) != lower((uint8_t)name[i]))
                        {
                            match = 0;
                            break;
                        }
                    }
                }
            }
            if(!match)
            {
                match = short_name_is(e, name, length);
            }
            if(match)
            {
                found->cluster = ((uint32_t)rd16(e + 20) << 16) | rd16(e + 26);
                if(v->type != 32)
                {
                    found->cluster &= 0xFFFF;
                }
                found->size = rd32(e + 28);
                found->directory = (e[11] & 0x10) != 0;
                return FAT_OK;
            }
        }
        lfn_valid = 0;
    }
    return error != FAT_OK ? error : FAT_ERR_NOT_FOUND;
}

static int path_find(const struct fat_volume *v,
                     const char *path,
                     struct fat_entry *found)
{
    struct fat_entry at = { .cluster = v->type == 32 ? v->root_cluster : 0, .size = 0, .directory = 1 };

    for(;;)
    {
        size_t length = 0;
        int error;

        while(*path == '/' || *path == '\\')
        {
            path++;
        }
        if(*path == 0)
        {
            break;
        }
        while(path[length] != 0 && path[length] != '/' && path[length] != '\\')
        {
            length++;
        }
        if(!at.directory)
        {
            return FAT_ERR_NOT_FOUND;
        }
        error = dir_find(v, at.cluster, path, length, &at);
        if(error != FAT_OK)
        {
            return error;
        }
        if(at.directory && at.cluster == 0 && v->type == 32)
        {
            at.cluster = v->root_cluster;
        }
        path += length;
    }

    *found = at;
    return FAT_OK;
}

int fat_size(struct fat_volume *volume,
             const char *path,
             uint64_t *size)
{
    struct fat_entry e;
    int error = path_find(volume, path, &e);

    if(error != FAT_OK)
    {
        return error;
    }
    if(e.directory)
    {
        return FAT_ERR_NOT_FILE;
    }
    *size = e.size;
    return FAT_OK;
}

int fat_load(struct fat_volume *volume,
             const char *path,
             void *buffer,
             uint64_t room,
             uint64_t *size)
{
    const struct fat_volume *v = volume;
    static uint8_t tail[FAT_MAX_SECTOR];
    struct fat_entry e;
    uint8_t *out = buffer;
    uint64_t left, clusters = 0;
    uint32_t cluster;
    int error;

    error = path_find(v, path, &e);
    if(error != FAT_OK)
    {
        return error;
    }
    if(e.directory)
    {
        return FAT_ERR_NOT_FILE;
    }
    if(size != NULL)
    {
        *size = e.size;
    }
    if(e.size > room)
    {
        return FAT_ERR_TOO_LARGE;
    }

    left = e.size;
    cluster = e.cluster;
    while(left > 0)
    {
        uint64_t first;
        uint32_t whole;

        if(cluster < 2 || cluster >= v->clusters + 2 || ++clusters > v->clusters)
        {
            return FAT_ERR_CORRUPT;
        }
        first = cluster_sector(v, cluster);

        if(left >= v->cluster_size)
        {
            error = fs_read(v, first, v->cluster_sectors, out);
            if(error != FAT_OK)
            {
                return error;
            }
            out += v->cluster_size;
            left -= v->cluster_size;
        }
        else
        {
            whole = (uint32_t)(left / v->sector_size);
            if(whole > 0)
            {
                error = fs_read(v, first, whole, out);
                if(error != FAT_OK)
                {
                    return error;
                }
                out += (uint64_t)whole * v->sector_size;
                left -= (uint64_t)whole * v->sector_size;
            }
            if(left > 0)
            {
                error = fs_read(v, first + whole, 1, tail);
                if(error != FAT_OK)
                {
                    return error;
                }
                copy(out, tail, (size_t)left);
                left = 0;
            }
            break;
        }

        if(left > 0)
        {
            cluster = fat_next(v, cluster);
        }
    }
    return FAT_OK;
}
