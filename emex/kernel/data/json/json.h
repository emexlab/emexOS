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

#ifndef JSON_H
#define JSON_H

#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/graph/lib/string.h>

//NOTE:
// this is a flat JSON parser with comments cuz i hate that default json doesnt have that.....

// i should prob make 0 limits just like the html parser but nah...
#define JSON_MAX_KEY 128
#define JSON_MAX_VALUE 256

typedef struct {
    char key[JSON_MAX_KEY];
    char value[JSON_MAX_VALUE];
} json_entry_t;

// loads flat JSON into entries array
int json_load(const char *path, json_entry_t *entries, int max_entries);
const char* json_get(json_entry_t *entries, int count, const char *key);

#endif
