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

#ifndef GEN_HEADER
#define GEN_HEADER


#include "../loader.h"


#include <kernel/user/user.h>
#include <kernel/user/ulime.h>
#include <kernel/proc/proc_manager.h>
#include <kernel/packages/elf/loader.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/communication/serial.h>
#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/include/reqs.h>
#include <kernel/graph/lib/string.h>
#include <kernel/graph/graphics.h>
#include <kernel/multitasking/multitasking.h>


#include <config/system.h>
#include <config/user.h>

#include <limine/limine.h>

#if ENABLE_ULIME
	extern proc_manager_t *proc_mgr;
	extern ulime_t *ulime;
	extern mt_t *mt;
#endif

void kproc(void);
void init_kernelprocesses2(void);
void uproc(void);

#endif
