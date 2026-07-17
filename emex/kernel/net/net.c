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

#include "net.h"
#include "eth.h"
#include "arp.h"
#include "ipv4.h"

#include <kernel/communication/serial.h>


#include <drivers/net/net.h>

int net_is_available(void)
{
	return netdrivers_available(); /* return 0 on failure */
}

/* qemu default address ig */
static const u8 g_default_ip[4] = { 10, 0, 2, 15 };

void netstack_init(void)
{
    eth_init();
    arp_init();
    //while (1)
    //{
    //    net_poll();
    //}
}
