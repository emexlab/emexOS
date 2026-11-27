/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.1
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.1
 * (EPSL 1.1).
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
#include <interrupt.h>

#define EMEX64_8042_REG_DATA    (EMEX64_8042_BASE + 0x00)
#define EMEX64_8042_REG_STATUS  (EMEX64_8042_BASE + 0x08)

#define EMEX64_8042_STATUS_OBF  0x01
#define EMEX64_8042_STATUS_AUX  0x20

void kbd_handler(struct fw_frame *frame)
{
    (void)frame;

    u64 status = mmio_read64(EMEX64_8042_REG_STATUS);
    if(!(status & EMEX64_8042_STATUS_OBF) || (status & EMEX64_8042_STATUS_AUX))
    {
        return;
    }

    fw_printf("[8042] unrecognized key event: scancode=0x%lx\n", mmio_read64(EMEX64_8042_REG_DATA));
}
