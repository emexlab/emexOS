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
 
OUTPUT_FORMAT("elf64-emex64")
ENTRY(_start)

SECTIONS
{
    . = 0;
    __image_start = .;
    .boot : {
        /*
         * the smallest header you have seen in your life:
         *
         * b _start
         */
        BYTE(0x26) BYTE(0x07) QUAD(_start)
    }
    __text_start = .;
    .text : { *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }
    . = ALIGN(0x2000);
    __data_start = .;
    .data : { *(.data .data.*) }
    .bss : {
        __bss_start = .;
        *(.bss .bss.*) *(COMMON)
        __bss_end = .;
    }
    __bss_size = __bss_end - __bss_start;
    __emex64_exec_img_end = .;

    /DISCARD/ : { *(.comment) *(.note.GNU-stack) *(.llvm_addrsig) }
}
