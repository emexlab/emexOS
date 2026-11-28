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

OUTPUT_ARCH(emex64)
ENTRY(_start)

SECTIONS
{
	. = 0;
	.text : {
		KEEP(*(.text.entry))
		*(.text .text.*)
		*(.rodata .rodata.*)
	}
	.dynsym : ALIGN(8) {
		HIDDEN(__dynsym_start = .);
		*(.dynsym)
		HIDDEN(__dynsym_end = .);
	}
	.dynstr : {
		HIDDEN(__dynstr_start = .);
		*(.dynstr)
		HIDDEN(__dynstr_end = .);
	}
	.hash     : { *(.hash) }
	.gnu.hash : { *(.gnu.hash) }
	.data : {
		*(.data .data.*)
		*(.got .got.plt)
	}
	.rela.dyn : {
		HIDDEN(__rela_dyn_start = .);
		*(.rela.dyn) *(.rela.*)
		HIDDEN(__rela_dyn_end = .);
	}
	.bss (NOLOAD) : ALIGN(64) {
		__bss_start = .;
		*(.bss .bss.*)
		*(COMMON)
		__bss_end = .;
	}
	.stack (NOLOAD) : ALIGN(16) {
		. += 0x10000;
		__stack_top = . - 8;
	}
	__eboot_end = .;
	/DISCARD/ : {
		*(.interp .dynamic)
		*(.note .note.*)
		*(.comment)
		*(.eh_frame .eh_frame_hdr)
	}
}
