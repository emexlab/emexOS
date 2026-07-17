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

#include <kernel/cpu/cpu.h>

// Disable interrupts
void cli(void) {
    __asm__ volatile("cli");
}

// Enable interrupts
void sti(void) {
    __asm__ volatile("sti");
}

// Full system halt
__attribute__((noreturn)) void chalt(void) {
    __asm__ volatile("cli");
    for (;;) __asm__ volatile("hlt");
}

void halt(void) {
    __asm__ volatile("hlt");
}

// Idle halt
__attribute__((noreturn)) void idle(void) {
    for (;;) __asm__ volatile("hlt");
}

// Wait for interrupt
void wfi(void) {
    __asm__ volatile("sti; pause; hlt");
}

void nop(void) {
	__asm__ volatile("nop");
}
