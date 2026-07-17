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

#ifndef NET_ETH_H
#define NET_ETH_H

// https://wiki.osdev.org/Ethernet

#include <types.h>

#define ETH_ALEN     6
#define ETH_HDR_LEN  14
#define ETH_MAX_LEN  1500

#define ETH_TYPE_ARP  0x0806
#define ETH_TYPE_IPV4 0x0800

typedef struct {
    u8     dst[ETH_ALEN];
    u8	   src[ETH_ALEN];
    u16    type;
} __attribute__((packed)) eth_frame_t;


void eth_init(void);
int eth_send(const u8 dst[ETH_ALEN], u16 type, const void *payload, u16 len);
void eth_recv(const void *frame, u16 len);
void eth_get_mac(u8 out[ETH_ALEN]);

#endif
