/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: halt.c
 * CREATED BY: asmileyguy
 * MODIFIED BY: emex
 *
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