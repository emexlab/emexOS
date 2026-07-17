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
