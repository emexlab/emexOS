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

#include "poweroff.h"

#include <types.h>
#include <kernel/graph/lib/string.h>
#include <kernel/graph/theme.h>

#include <kernel/include/ports.h>

//#define POWEROFF_SHUTDOWN 0
//#define POWEROFF_REBOOT   1

static inline void x86_restart(void)
{
    // Keyboard Controller Reset
    outb(0x64, 0xFE);

    for (volatile int i = 0; i < 1000000; i++)
        __asm__ volatile("nop");

    // PCI RCR
    outb(0xCF9, 0x02);
    for (volatile int i = 0; i < 100; i++) __asm__ volatile("nop");
    outb(0xCF9, 0x0E);
    for (volatile int i = 0; i < 1000000; i++) __asm__ volatile("nop");

    __asm__ volatile(
        "lidt %0\n"
        "int $3\n"
        :: "m"((u64[]){0, 0}) : "memory"
    );
}

static inline void x86_shutdown(void)
{
    // QEMU / Bochs
    outw(0x604, 0x2000);

    for (volatile int i = 0; i < 10000000; i++)
        __asm__ volatile("nop");

    // fallback
    outw(0xB004, 0x2000);
    outw(0x0604, 0x2000);

    for (volatile int i = 0; i < 10000000; i++)
        __asm__ volatile("nop");
}

__attribute__((visibility("default")))
void cpu_poweroff(int operation)
{
#if defined(__x86_64__)
    __asm__ volatile("cli" ::: "memory");

    if (operation == POWEROFF_REBOOT)
    {
        x86_restart();
    }
    else if (operation == POWEROFF_SHUTDOWN)
    {
        x86_shutdown();
    }
    else
    {
        print("Power operation not supported.\n", white());
        return;
    }

    print("Power operation failed or not supported by hardware.\n", white());
    return;

    //while (1)
    //    __asm__ volatile("hlt" ::: "memory");
#else
    print("Power management not supported on this architecture.\n", white());
    return;
#endif
}
