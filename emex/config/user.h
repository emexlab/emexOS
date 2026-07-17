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

#ifndef CONFIG_USER_H
#define CONFIG_USER_H

#define USERINI_PATH "/emr/config/users.ini"
#define USERADRESSNUM "kman"
#define USERPROC "user"
#define USERSPACE 0x40004000
#include <kernel/kernel_processes/bootscreen/boot.h>
#define USER_SCREEN_MODE BS3 /*2 */
#define USERPRIORITY 200

#define PC_NAME "pc"
#define USER_NAME "emex"

#define DEFAULT_PWD "emex"

#define DEFAULT_KM "US"

#endif /* CONFIG_USER_H */
