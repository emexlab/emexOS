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
#include <display.h>
#include <interrupt.h>
#include <img4.h>

void fw_boot_main(void);

int main(void)
{
    struct img4_device *device = (struct img4_device *)&img4_this_device;
    device->ecid = mmio_read64(CPUID_REG_ECID);

    /* finally C, how long I waited for this moment */
    put_init();
    interrupt_init();
    interrupt_register_handler(EMEX64_IC_IRQ_EXCEPTION, exception_handler);

    /* really hoping it finds some shit like Linux and boots into it, imagine this works */
    fw_boot_main();

    for(;;)
    {
        fw_hlt();
    }
}
