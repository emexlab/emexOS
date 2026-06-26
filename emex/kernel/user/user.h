/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: user.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef SYSCALL_H
#define SYSCALL_H
#include "ulime.h"
//#include "calls.h"

//void _init_syscalls(ulime_t *ulime);
void _init_syscalls_table(ulime_t *ulime);

u64 syscall_handler(u64 syscall_num, u64 arg1, u64 arg2, u64 arg3);

void JumpToUserspace(ulime_proc_t *proc);

#endif
