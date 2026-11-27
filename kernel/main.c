/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
 *
 * Private use and modification are permitted under the license.
 * Redistribution of source or binaries, modified or unmodified,
 * requires prior written permission from emexLabs.
 *
 * External deployment is subject to the source-availability
 * requirements and other conditions of the license.
 *
 * Provided "AS IS", without warranty, to the extent permitted by law.
 *
 * See LICENSE for the full license terms.
 */


extern char __bss_start[] __attribute__((visibility("hidden")));
extern char __bss_end[] __attribute__((visibility("hidden")));
extern char __eboot_end[] __attribute__((visibility("hidden")));

void kernel_main(void *info)
{
    for(volatile char *p = __bss_start; p < __bss_end; p++)
    {
        *p = 0;
    }

    for(;;)
    {
        __asm__ __volatile__("hlt");
    }
}

__attribute__((naked, section(".text.entry")))
void _start(void)
{
    __asm__ __volatile__(
        "add r1, pc, __stack_top@pcrel\n\t"
        "mov sp, r1\n\t"
        "bl kernel_main@plt");
}
