/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: poweroff.h
 * CREATED BY: emex (used code from ArTicZera)
 * MODIFIED BY: --
 *
 */

#ifndef POWEROFF_H
#define POWEROFF_H

#define POWEROFF_SHUTDOWN 0
#define POWEROFF_REBOOT   1

void cpu_poweroff(int operation);

#endif
