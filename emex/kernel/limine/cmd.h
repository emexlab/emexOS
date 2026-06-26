/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: cmd.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once
#include <types.h>
#include <limine/limine.h>

void cmd_init(void);
int cmd_is(const char *val);
int cmd_has(const char *token);

const char *cmd_get(void); // token = "" if none

// services/install
struct limine_file *cmd_get_kernel_file(void);