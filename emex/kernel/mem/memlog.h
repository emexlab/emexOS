/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: memlog.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef MEMLOG_H
#define MEMLOG_H

#include <types.h>
#include <kernel/mem/mem.h>
#include <kernel/graph/lib/string.h>
#include <kernel/communication/serial.h>
#include <kernel/include/reqs.h>

void memlog_print_map(void);

#endif