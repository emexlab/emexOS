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

#include <interrupt.h>
#include <pmm.h>
#include <eboot_api.h>
#include <arch/emex64/random.h>

extern char __bss_start[] __attribute__((visibility("hidden")));
extern char __bss_end[] __attribute__((visibility("hidden")));

void kernel_main(void *info)
{
    for(volatile char *p = __bss_start; p < __bss_end; p++)
    {
        *p = 0;
    }

    printf("[Kernel] hello from kernelcache!\n");

    icm_init();
    pmm_init();

    void *meow = pmm_alloc(0x20);
    printf("allocated %p\n", meow);

    meow = pmm_alloc(0x20);
    printf("allocated %p\n", meow);

    meow = pmm_alloc(0x4000);
    printf("allocated %p\n", meow);

    meow = pmm_alloc(0x20);
    printf("allocated %p\n", meow);

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
