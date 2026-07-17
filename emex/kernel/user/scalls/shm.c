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
#include <kernel/ipc/ipc.h>

u64 scall_mmap(ulime_proc_t *proc, u64 addr, u64 length, u64 shm_id) {
    (void)addr;

    u32 pages = (u32)((length + 0xFFF) / 0x1000);
    if (pages == 0) pages = 1;

    if (shm_id == 0) {
        u64 id = shm_create(proc->ulime, pages);
        if (id == IPC_SHM_INVALID) return (u64)-1;
        void *virt = shm_attach(proc->ulime, proc, id);
        return virt == NULL ? (u64)-1 : (u64)virt;
    }

    void *virt = shm_attach(proc->ulime, proc, shm_id);
    return virt == NULL ? (u64)-1 : (u64)virt;
}

u64 scall_munmap(ulime_proc_t *proc, u64 addr, u64 length, u64 arg3) {
    (void)length; (void)arg3;
    if (addr == 0) return (u64)-1;

    for (int i = 0; i < IPC_MAX_SHM; i++) {
        //shm_detach handles "not found" gracefully
        //
        // TODO:
        // reverse lookup by virt addr
        (void)proc;
        break;
    }

    return 0;
}
