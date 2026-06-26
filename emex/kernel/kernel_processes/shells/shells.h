/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: shells.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include "../kernel/gen.h"
#include <kernel/graph/lib/string.h>

#define SYSTEMLOCATE2 "/emr/system/system.emx"
#define EPROMPT "/emergency_shell # "
#define RPROMPT "/recovery_shell # "
#define SHELL_FG_COLOR white()

void emergency_shell(void);
void recovery_shell(void);