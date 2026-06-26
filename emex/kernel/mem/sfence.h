/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: sfence.h
 * CREATED BY: emex
 * MODIFIED BY: Voxi0
 *
 */

#pragma once

static inline void store_fence()
{
    __asm__ volatile("sfence" ::: "memory");
}