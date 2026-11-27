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

/* some hardware definitions and rules */
#define EMEX64_PAGE_SIZE            0x2000
#define EMEX64_MMIO_BASE            0x0020000000000000


/* needed MMIO constants */
#define EMEX64_MC_BASE              (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 5))
#define EMEX64_PLATFORM_BASE        (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 6))
#define EMEX64_MC_REG_SIZE          EMEX64_MC_BASE
#define EMEX64_PLATFORM_REG_PWR     EMEX64_PLATFORM_BASE

/* the early stack  */
    .section .bss
    .balign 16
early_stack:
    .space (EMEX64_PAGE_SIZE * 4)
early_stack_top:

/* this stays =3 */
.section .text
.globl _start
_start:
    /*
     * will just shutdown for now
     * if it does and your reading this
     * the reason is that there is not
     * enough space for the bss section
     */
    ldq r0, EMEX64_MC_REG_SIZE
    cbltu r0, __bss_end, .poweroff@pcrel

    /* zeroing the whole bss */
    setm __bss_start, 0, __bss_size

    /* early stack fixup */
    mov sp, early_stack_top - 8

    /*
     * branch and linking to the main symbol
     * of the bootloader :3 (hopefully does not
     * return)
     */
    bl main@pcrel
.poweroff:
    /* telling platform device to turn off */
    stq EMEX64_PLATFORM_REG_PWR, 0
