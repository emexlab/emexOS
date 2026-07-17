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

#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <types.h>
#include <config/user.h>

typedef struct {
    char pc_name[64];
    char user_name[64];
    char password[64];
    char keymap[64];
} user_config_t;


void uci(void);
int uci_save(void);
int uci_load(void);

// Getters for current config
const char* uci_get_pc_name(void);
const char* uci_get_user_name(void);
const char* uci_get_password(void);
const char* uci_get_keymap(void);

// Reload config (for use after file changes)
void uci_reload(void);

#endif
