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

#include "scalls.h"
#include <kernel/arch/x86_64/poweroff.h>
#include <kernel/user/ulime.h>
#include <drivers/ps2/mouse/mouse.h>

// unix/linux-like
#define RSYSTEM_MAGIC1       0xfee1dead
#define RSYSTEM_MAGIC2       0x28121969
#define RSYSTEM_CMD_RESTART  0x01234567
#define RSYSTEM_CMD_HALT     0xcdef0123
#define RSYSTEM_CMD_POWEROFF 0x4321fedc

u64 scall_reboot(ulime_proc_t *proc, u64 magic1, u64 magic2, u64 cmd) {
    (void)proc;

    if (magic1 != RSYSTEM_MAGIC1 || magic2 != RSYSTEM_MAGIC2) return (u64)-1;

    if (cmd == RSYSTEM_CMD_RESTART) {
        cpu_poweroff(POWEROFF_REBOOT);
    } else if (cmd == RSYSTEM_CMD_POWEROFF || cmd == RSYSTEM_CMD_HALT) {
        cpu_poweroff(POWEROFF_SHUTDOWN);
    }

    return (u64)-1;
}
