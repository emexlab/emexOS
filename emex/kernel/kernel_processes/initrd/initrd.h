/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: initrd.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef INITRD_H
#define INITRD_H

#define INITRD_MOUNT_POINT "/"
#define INITRD_MODULE_NAME "initrd.cpio"

int initrd_load(void);

#endif
