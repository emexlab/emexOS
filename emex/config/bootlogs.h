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

#ifndef CONFIG_BOOTLOGS_H
#define CONFIG_BOOTLOGS_H

#include "system.h"

#if BS_DEBUG == 1
#   define DBS1 1
#   define DBS2 1
#   define DBS3 1
#   define DBS4 1
#elif BS_DEBUG == 0
#   define DBS1 0
#   define DBS2 0
#   define DBS3 1
#   define DBS4 1
#endif

//BOOTUP_VISUALS config
#if BOOTUP_VISUALS == 0 // verbose boot
    #define BOOTUP_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
    #define BOOTUP_PRINT(msg, col) \
        do { \
            print(msg, col); \
        } while(0)
    #define BOOTUP_PRINT_INT(num, col) \
        do { \
            printInt(num, col); \
        } while(0)
    #define BOOTUP_PRINTBS(msg, col) \
        do { \
            printbs(msg, col); \
        } while(0)

#else     // silent boot
    #define BOOTUP_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
    #define BOOTUP_PRINT(msg, col)
    #define BOOTUP_PRINT_INT(num, col)
    #define BOOTUP_PRINTBS(msg, col)

#endif

#endif /* CONFIG_BOOTLOGS_H */
