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

#define TGZ_MAX_BITS 15
#define TGZ_OK 0
#define TGZ_ERR_FORMAT (-1)

typedef struct {
    const u8 *in;
    u8 *out;
    u8 *out_start;
    u32 bit_buf;
    int bit_cnt;
} tgz_stream;

int ungzip(const void *src, void *dst);
