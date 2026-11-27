/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
 *
 * Private use and modification are permitted under the license.
 * Redistribution of source or binaries, modified or unmodified,
 * requires prior written permission from emexLabs.
 *
 * External deployment is subject to the source-availability
 * requirements and other conditions of the license.
 *
 * Provided "AS IS", without warranty, to the extent permitted by law.
 *
 * See LICENSE for the full license terms.
 */

#include <fw.h>
#include <blk.h>

static struct blkdev *devices[BLK_MAX_DEVICES];
static int device_count;

void blk_register(struct blkdev *b)
{
    if(device_count < BLK_MAX_DEVICES)
    {
        devices[device_count++] = b;
    }
}

int blk_count(void)
{
    return device_count;
}

struct blkdev *blk_get(int index)
{
    return index >= 0 && index < device_count ? devices[index] : NULL;
}
