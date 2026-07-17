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

#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <types.h>
/*#include <limine/limine.h>

// limine reqs
__attribute__((used, section(".requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(3);

// framebuffer
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = NULL
    };*/
    //** moved to reqs.c/.h **

// halt
static inline void hcf(void)
{
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// simple delay
static inline void delay(u32 count)
{
    for (volatile u32 i = 0; i < count * 10000000; i++)
    {
        __asm__ volatile ("nop");
    }
    //maybe i add apic and timer later
}


#endif
