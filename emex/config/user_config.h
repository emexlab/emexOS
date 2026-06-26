/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: user_config.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
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
