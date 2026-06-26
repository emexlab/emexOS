/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: print.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef PRINT_H
#define PRINT_H

#include <types.h>

#include "console/console.h"
#define SCREENMODE BS1

void putchar_bootstrap(char c, u32 color);
void printbs(const char *str, u32 color);

// text output functions
void putchar(char c, u32 color);
void string(const char *str, u32 color);
void print(const char *str, u32 color);
void printInt(int value, u32 color);
void printInt_to(int screen, int value, u32 color);
void print_to(int screen, const char *str, u32 color);
//void cprintf(const char *str, u32 color);

void reset_cursor(void);

#endif
