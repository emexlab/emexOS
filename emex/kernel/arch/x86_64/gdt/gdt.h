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

#ifndef GDT_H
#define GDT_H

#include <types.h>

#define GDT_ENTRIES 7

// GDT entry flags
#define GDT_PRESENT     0x80
#define GDT_RING0       0x00
#define GDT_RING3       0x60
#define GDT_SYSTEM      0x00
#define GDT_CODE_DATA   0x10
#define GDT_EXECUTABLE  0x08
#define GDT_RW          0x02
#define GDT_ACCESSED    0x01

// GDT granularity flags
#define GDT_GRANULAR    0x80  // 4KB blocks
#define GDT_LONG_MODE   0x20  // 64-bit
#define GDT_32BIT       0x40  // 32-bit protected mode

// segment selectors
#define KERNEL_CODE_SELECTOR  0x08
#define KERNEL_DATA_SELECTOR  0x10
#define USER_DATA_SELECTOR    0x18
#define USER_CODE_SELECTOR    0x20
#define TSS_SELECTOR          0x28

// GDT entry structure (64-bit)
typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed)) gdt_entry_t;

// TSS entry structure (64-bit)
typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  granularity;
    u8  base_high;
    u32 base_upper;
    u32 reserved;
} __attribute__((packed)) gdt_tss_entry_t;

// GDT ptr structure
typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) gdt_ptr_t;

// TSS
typedef struct {
    u32 reserved0;
    u64 rsp0;  // kernel stack pointer
    u64 rsp1;
    u64 rsp2;
    u64 reserved1;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 reserved2;
    u16 reserved3;
    u16 iopb_offset;
} __attribute__((packed)) tss_t;



void gdt_init(void);
void gdt_set_kernel_stack(u64 stack);
void gdt_load(void);
void tss_init(void);
void tss_set_stack(u64 stack);
void verify_gdt_setup(void);


#endif
