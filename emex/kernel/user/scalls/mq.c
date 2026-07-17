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
#include <kernel/user/system/ptrs.h>

u64 scall_mq_open(ulime_proc_t *proc, u64 name_ptr, u64 oflag, u64 mode)
{
    (void)oflag;
    (void)mode;

    if (!is_valid_user_ptr(name_ptr)) return (u64)-1;

    open_inbox(proc->ulime, proc);
    return (u64)proc->pid;
}

u64 scall_mq_unlink(ulime_proc_t *proc, u64 name_ptr, u64 arg2, u64 arg3)
{
    (void)name_ptr;
    (void)arg2;
    (void)arg3;

    destroy_endpoint(proc->ulime, proc);
    return 0;
}

u64 scall_mq_send(ulime_proc_t *proc, u64 mqid, u64 buf, u64 size) {
	int r = send_msg(proc->ulime, proc, mqid, (const void *)buf, (u32)size);

    if (!is_valid_user_ptr(buf)) return (u64)-1;
    if (size == 0 || size > IPC_MAX_MSG_SIZE) return (u64)-1;

    return r == 0 ? 0 : (u64)-1;
}

u64 scall_mq_recv(ulime_proc_t *proc, u64 mqid, u64 buf, u64 size)
{
    (void)mqid;

    u64 sender = 0;
    int got = receive_async_msg(proc->ulime, proc, (void *)buf, (u32)size, &sender);

    if (!is_valid_user_ptr(buf)) return (u64)-1;
    if (size == 0) return (u64)-1;
    if (got < 0) {
        receive_msg(proc->ulime, proc, (void *)buf, (u32)size, &sender);
        got = (int)size;
    }

    return (u64)got;
}
