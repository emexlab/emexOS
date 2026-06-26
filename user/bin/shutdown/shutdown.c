/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: shutdown.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <emx/system.h>
#include <unistd.h>

int main(void) {
    reboot(RSYSTEM_CMD_POWEROFF);
    _exit(1);
}