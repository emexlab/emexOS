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

#include "idt.h"
#include <kernel/arch/x86_64/exceptions/isr.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <kernel/mem/lib/main.h>
#include <kernel/graph/lib/string.h>
#include <kernel/kernel_processes/tm/cfg/stdclrs.h>
#include <kernel/graph/theme.h>


static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

extern void idt_flush(u64);

void idt_set_gate(u8 num, u64 handler, u8 flags)
{
    idt[num].offset_low = handler & 0xFFFF;
    idt[num].selector = 0x08; // kernel Code Segment
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].offset_mid = (handler >> 16) & 0xFFFF;
    idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].reserved = 0;
}

void idt_set_gate_ist(u8 num, u64 handler, u8 flags, u8 ist)
{
    idt_set_gate(num, handler, flags);
    idt[num].ist = ist & 0x07;
}

void idt_load(void)
{
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (u64)&idt;

    idt_flush((u64)&idt_ptr);
}

void idt_init(void)
{
    log("[IDT]", "Init interrupts\n", d);
    memset(&idt, 0, sizeof(idt)); // clear idt
    isr_install();
    irq_install();
    idt_load();
}
