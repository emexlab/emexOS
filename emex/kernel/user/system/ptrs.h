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

#define KSTART 0x0000800000000000ULL

static inline int is_valid_user_ptr(u64 pointer) {
	/* This should be used to chec k if a user-provided pointer
	 * is within the "safe" range and is not NULL.
	*/
	return pointer && pointer <= KSTART;
}

static inline int is_valid_user_ptr_range(u64 pointer, u64 size) {
	/* This should be used to check if a user-provided pointer and `size` ahead of it
	 * is within the "safe" range and is not NULL.
	*/
	if (size == 0) return 0;
	if (size == 1) return is_valid_user_ptr(pointer);

	u64 max;
	if (__builtin_add_overflow(pointer, size, &max))
		return 0;

	return (pointer && pointer <= KSTART) && (max && max <= KSTART);
}
