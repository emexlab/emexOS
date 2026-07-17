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

#ifndef NET_ARP_H
#define NET_ARP_H

// https://wiki.osdev.org/Address_Resolution_Protocol

#include <types.h>
#include "eth.h"

#define ARP_OP_REQUEST   1
#define ARP_OP_REPLY     2

#define ARP_TABLE_SIZE 16

typedef struct {
    u16    hw_type;   /* 1 == ethernet */
    u16    proto;     /* 0x0800 == ipv4 */
    u8     hw_len;     /* 6 */
    u8     proto_len;  /* 4 */
    u16    op;
    u8    sender_mac[ETH_ALEN];
    u8    sender_ip[4];
    u8    target_mac[ETH_ALEN];
    u8    target_ip[4];
} __attribute__((packed)) arp_packet_t;
typedef struct {
    u8    ip[4];
    u8    mac[ETH_ALEN];
    int    valid;
} arp_entry_t;


void arp_init(void);

int  arp_request(const u8 ip[4]);
int  arp_lookup(const u8 ip[4], u8 mac_out[ETH_ALEN]);

void arp_recv(const void *packet, u16 len);

#endif
