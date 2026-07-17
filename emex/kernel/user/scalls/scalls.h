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

#pragma once

#include <types.h>
#include <kernel/user/ulime.h>

u64 scall_mq_open(ulime_proc_t *proc, u64 name_ptr, u64 oflag, u64 mode);
u64 scall_mq_unlink(ulime_proc_t *proc, u64 name_ptr, u64 arg2, u64 arg3);
u64 scall_mq_send(ulime_proc_t *proc, u64 mqid, u64 buf, u64 size);
u64 scall_mq_recv(ulime_proc_t *proc, u64 mqid, u64 buf, u64 size);

u64 scall_mkdir(ulime_proc_t *proc, u64 path_ptr, u64 a2, u64 a3);
u64 scall_unlink(ulime_proc_t *proc, u64 path_ptr, u64 a2, u64 a3);

u64 scall_mmap(ulime_proc_t *proc, u64 addr, u64 length, u64 shm_id);
u64 scall_munmap(ulime_proc_t *proc, u64 addr, u64 length, u64 arg3);

//u64 scall_fork(ulime_proc_t *proc, u64 a1, u64 a2, u64 a3);

// emex specific
u64 scall_reboot(ulime_proc_t *proc, u64 magic1, u64 magic2, u64 cmd);
u64 scall_sysinfo(ulime_proc_t *proc, u64 info_addr, u64 a1, u64 a2);
