/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: bootlogs.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

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
