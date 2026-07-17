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

#ifndef DEVICE_URANDOM_H
#define DEVICE_URANDOM_H

#include <kernel/module/module.h>

// /dev/urandom
extern driver_module urandom_module;

// shared with /dev/random alias
void *urandom_open_fn(const char *path);
int urandom_read_fn(void *handle, void *buf, size_t count, u64 offset);
int urandom_write_fn(void *handle, const void *buf, size_t count, u64 offset);

#endif
