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

#ifndef POWEROFF_H
#define POWEROFF_H

__attribute__((noreturn))
static void power_off(void)
{
    __asm__ __volatile__("stq 0x2000000000c000, 0" : : : "memory");
    for(;;)
    {
        __asm__ __volatile__("hlt");
    }
}

#endif /* POWEROFF_H */
