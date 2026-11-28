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

#include <icm.h>
#include <mm/pmm.h>
#include <mm/pmap.h>
#include <eboot_api.h>
#include <arch/emex64/random.h>

extern char __bss_start[] __attribute__((visibility("hidden")));
extern char __bss_end[] __attribute__((visibility("hidden")));

void kernel_main(struct e64_boot_info *info)
{
    memset(__bss_start, 0, ((size_t)__bss_end - (size_t)__bss_start));

    printf("[Kernel] hello from kernelcache!\n");

    icm_init();
    pmm_init();
    if(info->fb_base != 0 && info->fb_stride != 0 && info->fb_height != 0)
    {
        /* prevents the screen from flickering (yk since the fb could be allocated it self) */
        uint64_t fb_size = (uint64_t)info->fb_stride * (uint64_t)info->fb_height;
        pmm_reserve_range(info->fb_base, fb_size);
    }
    pmap_bootstrap();

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
