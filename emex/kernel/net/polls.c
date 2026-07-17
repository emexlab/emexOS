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

//#include <drivers/net/e1000/e1000.h>
#include <kernel/communication/serial.h>
#include "net.h"
#include "eth.h"
#include <drivers/net/net.h>
#include <kernel/graph/lib/string.h>
#include <kernel/kernel_processes/bootscreen/print.h>
#include <kernel/kernel_processes/bootscreen/log.h>

void net_poll(void)
{
	void* data;

	//while ((len = e1000_recv(&data)))
	//{
	//	printf("packet len=%d\n", len);
	//}
 	static u8 buf[1518];
    int len;

    while ((len = netdrivers_recv(buf, sizeof(buf))) > 0)
    {
        logf(d, "NET", "packet received, len=%d", len);

        eth_recv(buf, (u16)len);
    }
}
