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

#ifndef NET_IPV4_H
#define NET_IPV4_H

// https://wiki.osdev.org/IPv4

#include <types.h>
#include "eth.h"

#define IPV4_PROTO_ICMP  	1 /*make ping */
#define IPV4_PROTO_TCP   	6
#define IPV4_PROTO_UDP   17

#define IPV4_TTL_DEFAULT 64
#define IPV4_HDR_LEN 20

typedef struct
{
    u8  	version_ihl;
    u8  	protocol;
    u8  	src[4];
    u8  	dst[4];
} __attribute__((packed)) ipv4_hdr_t;


#endif
