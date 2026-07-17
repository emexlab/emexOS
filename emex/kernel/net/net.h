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

// gen net

#define _EU_UNIX 0
#define _EU_P 1
#include <config/tasks.h>
//#include <kernel/net/pipe/pipe.h>

#define NET_MAC_LEN 6
#define NET_IP_LEN  4

void netstack_init();

#include <drivers/net/net.h>
int net_is_available(void);

// initial tasks
#ifndef SOCKET1
#   include <head.h>
#	define EITASK "1"
#elif SOCKET2
#	define EITASK "2"
#elif SOCKET3
#	define EITASK "3"
#elif SOCKET4
#	define EITASK "4"
#endif

void net_poll();
