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

#ifndef NETLAYER
#define NETLAYER

#include <types.h>

typedef struct
{
    int  	(*init) (void); /* initialize network drivers */
    int  	(*send) (const void *data, u16 len); /* send data */
    int  	(*recv) (void *buf, u16 max_len); /* recive data */
    void 	(*get_mac)(u8 mac[6]);
} netdrivers_driver_t;

int netdrivers_init(void);
int netdrivers_send(const void *data, u16 len);
int netdrivers_recv(void *buf, u16 max_len);

void netdrivers_get_mac(u8 mac[6]);

#endif
