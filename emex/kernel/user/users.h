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

#ifndef USERS_H

#define USERS_H

#include <types.h>

#define USERS_MAX      32
#define USER_NAME_MAX  64
#define USER_HOME_MAX 128

#define UID_ROOT 		0
#define GID_ROOT 		0

typedef struct
{
    char 	username[USER_NAME_MAX];
    u32  	uid;
    u32  	gid;
    char 	home[USER_HOME_MAX];
    u8   	valid;
} user_entry_t;

// vfs permisssion chec
extern u32 g_current_uid;
extern u32 g_current_gid;

void users_init(void);
void users_setup_homes(void);

int users_load(const char *path);

u32 users_get_count(void);

user_entry_t *users_get_by_uid(u32 uid);
user_entry_t *users_get_by_name(const char *name);
user_entry_t *users_get_first_regular(void);

#endif
