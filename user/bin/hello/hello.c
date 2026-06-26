/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: hello.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <unistd.h>
#include <stdio.h>

#include <emx/ansi.h>

int main(void)
{
    printf(A_GFX_GREEN "\nhello, world!\n\n");
    return 0;
}
