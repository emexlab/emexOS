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

#include "proc_manager.h"

#include <kernel/mem/lib/main.h>
#include <kernel/mem/klime/klime.h>

// rn this only works if ulime is on

proc_manager_t* proc_mng_init(ulime_t *ulime)
{
    if (!ulime || !ulime->klime) return NULL;
    proc_manager_t *pm = (proc_manager_t*)klime_create(ulime->klime, sizeof(proc_manager_t));
    if (!pm) return NULL;
    pm->ulime = ulime;
    return pm;
}

ulime_proc_t* proc_create_proc(proc_manager_t *pm, u8 *name, u64 entry, u64 priority)
{
    if (!pm) return NULL;

    ulime_proc_t *proc = ulime_proc_create(pm->ulime, name, entry);
    if (!proc) return NULL;

    proc->priority = priority; // processes have priority

    // map memory
    if (ulime_proc_mmap(pm->ulime, proc) != 0) {
        ulime_proc_kill(pm->ulime, proc->pid);
        return NULL;
    }

    return proc;
}

int proc_kill_proc(proc_manager_t *pm, u64 pid) {
    if (!pm) return -1;

    return ulime_proc_kill(pm->ulime, pid);
}

int proc_load_program_from_ulime(proc_manager_t *pm, ulime_proc_t *proc, u8 *code, u64 size) {
    if (!pm || !proc) return -1;

    //return ulime_load_program(proc, code, size);
    return 0;
}

void proc_list_procs(proc_manager_t *pm) {
    if (!pm) return;

    // only if ulime is on
    ulime_proc_list(pm->ulime);
}

ulime_proc_t* proc_find(proc_manager_t *pm, u64 pid)
{
    if (!pm) return NULL;

    ulime_proc_t *p = pm->ulime->ptr_proc_list;
    while (p) {
        if (p->pid == pid) return p;
        p = p->next;
    }

    return NULL;
}

int proc_count(proc_manager_t *pm)
{
    if (!pm) return 0;

    int count = 0;
    ulime_proc_t *p = pm->ulime->ptr_proc_list;

    while (p) {
        count++;
        p = p->next;
    }

    return count;
}
