/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: serial.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <types.h>

// addresses
#define COM1 0x3F8
#define COM2 0x2F8

void serial_init(void);
int serial_ready(void);

void serial_putchar(char c);
void serial_puts(const char *str);
void serial_printf(const char *format, ...);

// allias
#define printf serial_printf

#endif
