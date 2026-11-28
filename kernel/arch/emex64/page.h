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

#ifndef __ARCH_EMEX64_PAGE_H
#define __ARCH_EMEX64_PAGE_H

#define EMEX64_PAGE_SIZE            0x2000
#define EMEX64_PAGE_MASK            (EMEX64_PAGE_SIZE - 1)
#define EMEX64_PAGE_ROUND_DOWN(x)   ((x) & ~((EMEX64_PAGE_SIZE) - 1))
#define EMEX64_PAGE_ROUND_UP(x)     (((x) + (EMEX64_PAGE_SIZE) - 1) & ~((EMEX64_PAGE_SIZE) - 1))
#define EMEX64_IS_ALIGNED_64(addr)  (((addr) & 0x7) == 0)

#endif /* __ARCH_EMEX64_PAGE_H */
