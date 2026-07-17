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

/* This implements hardware-backed random number generation, utilizing the
 * RDTSC instruction (available on all x64 CPUs afaik).
 *
 * This was initially made for urandom, since it was relying on Xorshift64.
*
 * NOTE: Make sure to check `supports_hardware_random()` before calling get_hw_rand! The function itself will make no attempt to check for it and _WILL_ cause a #UD!
*/

#ifndef RAND_H
#define RAND_H

#include <kernel/cpu/cpu.h>

u64 rdtsc(void)
{
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi << 32) | lo;
}

u64 get_hw_rand(void)
{
	u64 value = rdtsc();
	for (int i = 0; i < 8 && value == 0; i++)
		value = rdtsc();

	if (value == 0) value = 0xD3ADF00D;

	value ^= (u64) &value; // AUDITME: We're using the stack address as an extra source of entropy, but hopefully the steps below are enough to make address leakage impossible (or atleast very, _VERY_ impractical).

	// Extra scrambling work. I hope this is enough.
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9;
    value = (value ^ (value >> 27)) * 0x94d049bb133111eb;

	return value ^ (value >> 31);
}

#endif
