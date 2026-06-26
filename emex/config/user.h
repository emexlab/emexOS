/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: user.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */
#pragma once

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
