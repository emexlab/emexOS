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
#include <kernel/mem/klime/klime.h>
#include <kernel/mem/paging/paging.h>

//
// https://github.com/Novice06/Novix/tree/main/src/kernel/multitasking/ipc
//

#define IPC_MAX_MESSAGES  256
#define IPC_MAX_MSG_SIZE  512
#define IPC_MAX_PROCS     32
#define IPC_MAX_WAITERS   16
#define IPC_MAX_SHM       64
#define IPC_MAX_SHM_PAGES 64    // 256KB per region max
#define IPC_SHM_INVALID   0

typedef struct {
    u64 sender_id;
    u64 receiver_id;
    u32 size;
    u8 data[IPC_MAX_MSG_SIZE];
    int used;
} ipc_msg_t;

typedef struct {
    int is_open;
    int count;
    ipc_msg_t *messages[IPC_MAX_MESSAGES];
    ulime_proc_t *waiters[IPC_MAX_WAITERS];
    int waiter_count;
} ipc_port_t;

typedef struct {
    u64 id;
    u64 phys_pages[IPC_MAX_SHM_PAGES];
    u32 page_count;
    int ref_count;
    int used;
    u64 virt_per_proc[IPC_MAX_PROCS];
} ipc_shm_t;

void ipc_messages_init(void);
void ipc_shm_init(void);

void open_inbox(ulime_t *ulime, ulime_proc_t *proc);
void close_inbox(ulime_proc_t *proc);
void destroy_endpoint(ulime_t *ulime, ulime_proc_t *proc);
int send_msg(ulime_t *ulime,ulime_proc_t *sender,u64 receiver_id,const void *data,u32 size);
int receive_async_msg(ulime_t *ulime,ulime_proc_t *proc,void *buf,u32 buf_size,u64 *out_sender);
void receive_msg(ulime_t *ulime,ulime_proc_t *proc,void *buf,u32 buf_size,u64 *out_sender);

u64 shm_create(ulime_t *ulime, u32 pages);
void *shm_attach(ulime_t *ulime, ulime_proc_t *proc, u64 id);
void shm_detach(ulime_t *ulime, ulime_proc_t *proc, u64 id);
void shm_detach_all(ulime_t *ulime, ulime_proc_t *proc);
