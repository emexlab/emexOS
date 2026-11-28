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

PHDRS
{
	text    PT_LOAD    FLAGS(5);
	rodata  PT_LOAD    FLAGS(4);
	data    PT_LOAD    FLAGS(6);
	dynamic PT_DYNAMIC FLAGS(6);
}

SECTIONS
{
	. = 0;
	.text : ALIGN(0x1000) {
		KEEP(*(.text.entry))
		*(.text .text.*)
	} :text
	.rodata : ALIGN(0x1000) {
		*(.rodata .rodata.*)
	} :rodata
	.dynsym   : { *(.dynsym) }   :rodata
	.dynstr   : { *(.dynstr) }   :rodata
	.hash     : { *(.hash) }     :rodata
	.gnu.hash : { *(.gnu.hash) } :rodata
	.rela.dyn : {
		*(.rela.dyn) *(.rela.*) *(.relr.dyn)
	} :rodata
	.data : ALIGN(0x1000) {
		*(.data .data.*)
	} :data
	.dynamic : {
		*(.dynamic)
	} :data :dynamic
	.got : {
		*(.got .got.*)
	} :data
	.bss (NOLOAD) : ALIGN(64) {
		HIDDEN(__bss_start = .);
		*(.bss .bss.*)
		*(COMMON)
		HIDDEN(__bss_end = .);
	} :data
	.stack (NOLOAD) : ALIGN(16) {
		. += 0x10000;
		HIDDEN(__stack_top = . - 8);
	} :data
	HIDDEN(__kernel_end = .);
	/DISCARD/ : {
		*(.interp)
		*(.note .note.*)
		*(.comment)
		*(.eh_frame .eh_frame_hdr)
	}
}
