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

#ifndef CONF_H
#define CONF_H

#define CONF_MAX_KEY 64
#define CONF_MAX_VALUE 256

typedef struct {
    char key[CONF_MAX_KEY];
    char value[CONF_MAX_VALUE];
} conf_entry_t;

int conf_load(const char *path, conf_entry_t *entries, int max_entries);
const char* conf_get(conf_entry_t *entries, int count, const char *key);

#endif
