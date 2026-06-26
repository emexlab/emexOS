/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: info.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include <config/system.h>

#include "config.h"
#define LOGO_MODULE_NAME ISLPATH
#define LOGO_MIN_SIZE    9  // 8 header bytes + at least 1 pixel

void bs2_draw_info(void);