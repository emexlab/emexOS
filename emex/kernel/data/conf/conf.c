/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * emexOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * emexOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with emexOS. If not, see <https://www.gnu.org/licenses/>.
 */

#include "conf.h"
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/mem/lib/main.h>
#include <kernel/graph/lib/string.h>

// src/kernel/data/conf.c

int conf_load(const char *path, conf_entry_t *entries, int max_entries) {
    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) return -1;

    char buf[2048];
    ssize_t bytes = fs_read(fd, buf, sizeof(buf) - 1);
    fs_close(fd);

    if (bytes <= 0) return -1;
    buf[bytes] = '\0';

    int count = 0;
    const char *line = buf;

    while (*line && count < max_entries) {
        while (*line == ' ' || *line == '\t') line++;

        if (*line == '#' || *line == '\n') {
            while (*line && *line != '\n') line++;
            if (*line) line++;
            continue;
        }

        const char *key_start = line;
        while (*line && *line != ':' && *line != '\n') line++;

        if (*line != ':') {
            while (*line && *line != '\n') line++;
            if (*line) line++;
            continue;
        }

        int key_len = line - key_start;
        if (key_len >= sizeof(entries[count].key)) key_len = sizeof(entries[count].key) - 1;

        for (int i = 0; i < key_len; i++) {
            entries[count].key[i] = key_start[i];
        }
        entries[count].key[key_len] = '\0';

        line++;
        while (*line == ' ' || *line == '\t') line++;

        const char *value_start = line;
        while (*line && *line != '\n') line++;

        int value_len = line - value_start;
        if (value_len >= sizeof(entries[count].value)) value_len = sizeof(entries[count].value) - 1;

        for (int i = 0; i < value_len; i++) {
            entries[count].value[i] = value_start[i];
        }
        entries[count].value[value_len] = '\0';

        count++;

        if (*line) line++;
    }

    return count;
}

const char* conf_get(conf_entry_t *entries, int count, const char *key) {
    for (int i = 0; i < count; i++) {
        if (str_equals(entries[i].key, key)) {
            return entries[i].value;
        }
    }
    return NULL;
}
