/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: fs.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "scalls.h"
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/graph/lib/string.h>


u64 scall_mkdir(ulime_proc_t *proc, u64 path_ptr, u64 arg2, u64 arg3) {
    (void)proc; (void)arg2; (void)arg3;
    if (!path_ptr || path_ptr > 0x0000800000000000ULL) return (u64)-1;
    int r = fs_mkdir((const char *)path_ptr);
    return (r < 0) ? (u64)-1 : 0;
}

u64 scall_unlink(ulime_proc_t *proc, u64 path_ptr, u64 arg2, u64 arg3) {
    (void)proc; (void)arg2; (void)arg3;
    if (!path_ptr || path_ptr > 0x0000800000000000ULL) return (u64)-1;
    int r = fs_unlink((const char *)path_ptr);
    return (r < 0) ? (u64)-1 : 0;
}