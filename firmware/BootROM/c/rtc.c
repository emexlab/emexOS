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

#define EMEX64_RTC_REG_SECONDS  (EMEX64_RTC_BASE + 0x00)
#define EMEX64_RTC_REG_MINUTES  (EMEX64_RTC_BASE + 0x08)
#define EMEX64_RTC_REG_HOURS    (EMEX64_RTC_BASE + 0x10)
#define EMEX64_RTC_REG_DAY      (EMEX64_RTC_BASE + 0x18)
#define EMEX64_RTC_REG_MONTH    (EMEX64_RTC_BASE + 0x20)
#define EMEX64_RTC_REG_YEAR     (EMEX64_RTC_BASE + 0x28)
#define EMEX64_RTC_REG_WEEKDAY  (EMEX64_RTC_BASE + 0x30)

void print_time(void)
{
    fw_printf("[fw] date: %02lu.%02lu.%02lu %02lu:%02lu:%02lu\n",
              mmio_read64(EMEX64_RTC_REG_DAY),
              mmio_read64(EMEX64_RTC_REG_MONTH),
              mmio_read64(EMEX64_RTC_REG_YEAR),
              mmio_read64(EMEX64_RTC_REG_HOURS),
              mmio_read64(EMEX64_RTC_REG_MINUTES),
              mmio_read64(EMEX64_RTC_REG_SECONDS));
}
