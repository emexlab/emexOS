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

#ifndef ULIME_H
#define ULIME_H

#include <types.h>
#include <kernel/mem/mem.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/mem/phys/physmem.h>
#include "system/calls.h"

#define NAMESPACE 14
#define PIDSPACE 4
#define STATSPACE 10


typedef struct ulime_proc {
    u64 pid;
    u8  name[64];
    u64 priority;
    u64 state;
    u64 *ptr_pagetable;
    u64 pml4_phys;    // physical address of this process's PML4 (loaded into CR3)

    u64 entry_point;

    u64 heap_base;
    u64 heap_size;
    u64 stack_base;
    u64 stack_size;
    u64 code_base;
    u64 code_size;

    u64 phys_code;
    u64 phys_heap;
    u64 phys_stack;

    u64 brk;
    u64 entry_rsp;
    u64 mmap_base;    // next virtual address for mmap allocations

    u64 vma_base; // or. ELF VMA base

    // user context
    u32 uid;
    u32 gid;

    struct ulime *ulime;
    struct ulime_proc *next;
    struct ulime_proc *prev;
} ulime_proc_t;

typedef u64 (*syscall_handler_t)(ulime_proc_t *proc, u64 arg1, u64 arg2, u64 arg3);

typedef struct ulime_thread {
    u64 tid;
    ulime_proc_t *ptr_proc;
    u64 *ptr_stack;
    u64 *ptr_instruction;
    u64 state;
} ulime_thread_t;


typedef struct ulime {
    ulime_proc_t   *ptr_proc_list;
    ulime_proc_t   *ptr_proc_curr;
    ulime_thread_t *ptr_thread;
    u64 pid_next;
    u64 tid_next;

    klime_t *klime;
    limine_hhdm_response_t *hpr;

    syscall_handler_t syscalls[256];

    u64 internal_pool_base;
    u64 internal_pool_size;
    u64 internal_pool_used;

    // USER memory space management
    u64 user_space_base;
    u64 user_space_size;
    u64 user_space_used;

    //ipc
} ulime_t;


#define PROC_CREATED    1
#define PROC_READY      2
#define PROC_RUNNING    3
#define PROC_BLOCKED    4
#define PROC_ZOMBIE     5

ulime_t *ulime_init(
    limine_hhdm_response_t *hpr,
    klime_t *klime,
    u64 uphys_start
);

ulime_proc_t *ulime_proc_create(ulime_t *ulime, u8 *name, u64 entry_point);
int ulime_proc_kill(ulime_t *ulime, u64 pid);
int ulime_proc_mmap(ulime_t *ulime, ulime_proc_t *proc);
void ulime_proc_test_mem(ulime_proc_t *proc);

void ulime_schedule(ulime_t *ulime);
void ulime_proc_list(ulime_t *ulime);

int ulime_load_program(ulime_proc_t *proc, u8 *code, u64 code_size);

#endif
