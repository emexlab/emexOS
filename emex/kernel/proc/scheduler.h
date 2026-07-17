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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <types.h>
#include <kernel/user/ulime.h>

// how many ticks a process gets before we force a switch
// higher priority processes get more ticks (priority * SCHED_BASE_QUANTUM)
#define SCHED_BASE_QUANTUM 2

// minimum ticks a process always gets, even at lowest priority
#define SCHED_MIN_QUANTUM 1

// max ticks a process can get, so high priority cant starve others forever
#define SCHED_MAX_QUANTUM 16

// after this many rounds without running, bump a starving process to front
#define SCHED_STARVATION_LIMIT 64

typedef struct {
    ulime_t *ulime;

    u64 quantum;        // current quantum for the running task
    u64 ticks;          // ticks used so far this quantum

    int last_idx;       // last task index we ran (for round robin rotation)
    u64 round_counter;  // increments every time we complete a full round
} scheduler_t;

#if ENABLE_ULIME
extern scheduler_t *scheduler;
#endif

// init
scheduler_t* scheduler_init(ulime_t *ulime, u64 base_quantum);
void scheduler_set_quantum(scheduler_t *s, u64 quantum);

// called from timer to maybe do a context switch
void scheduler_tick(scheduler_t *s);

// called when a process wants to give up its remaining time
void scheduler_yield(scheduler_t *s);

// pick the next task index from the mt task list
// returns -1 if nothing is ready
// uses round robin but biased by priority
int scheduler_pick_next(scheduler_t *s, void *mt_ptr, int current_idx);

// compute how many ticks a process with given priority should get
u64 scheduler_quantum_for_priority(u64 priority);

#endif
