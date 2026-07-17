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

#include <kernel/communication/serial.h>

void verify_gdt_setup(void) {
    u16 cs, ss, ds, es;
    __asm__ volatile("mov %%cs, %0" : "=r"(cs));
    __asm__ volatile("mov %%ss, %0" : "=r"(ss));
    __asm__ volatile("mov %%ds, %0" : "=r"(ds));
    __asm__ volatile("mov %%es, %0" : "=r"(es));

    printf("[GDT-VERIFY] Current segments:\n");
    printf("  CS: 0x%04X (expected 0x0008 for kernel)\n", cs);
    printf("  SS: 0x%04X (expected 0x0010 for kernel)\n", ss);
    printf("  DS: 0x%04X\n", ds);
    printf("  ES: 0x%04X\n", es);

    // Check if user segments are valid by trying to load them
    printf("[GDT-VERIFY] Testing user segments:\n");
    __asm__ volatile(
        "mov $0x1B, %%ax\n"
        "mov %%ax, %%fs\n"//load into FS
        ::: "ax"
    );
    printf("  USER_DATA (0x1B): OK\n");
    u16 tr;
    __asm__ volatile("str %0" : "=r"(tr));
    printf("  Task Register: 0x%04X (expected 0x0028 for TSS)\n", tr);

    if (tr == 0) {
        printf("  ERROR: Task Register not loaded! TSS not active!\n");
    }
}
