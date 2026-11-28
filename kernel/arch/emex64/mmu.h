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

#ifndef __ARCH_EMEX64_MMU_H
#define __ARCH_EMEX64_MMU_H

/* MMU permission bits */
#define EMEX64_MMU_PT_PRESENT       0b00000001
#define EMEX64_MMU_PT_USER          0b00000010
#define EMEX64_MMU_PT_DIRTY         0b00000100
#define EMEX64_MMU_PT_READ          0b00001000
#define EMEX64_MMU_PT_WRITE         0b00010000
#define EMEX64_MMU_PT_EXEC          0b00100000
#define EMEX64_MMU_PT_ACCESSED      0b01000000
#define EMEX64_MMU_PT_MMIO          0b10000000

#define EMEX64_MMU_MASK_FLAGS       0b0000000000000000000000000000000000000000000000000000000011111111
#define EMEX64_MMU_MASK_PFN         0b1111111111111111111111111111111111111111111111111111111100000000

#define EMEX64_MMU_ASID_SHIFT       48
#define EMEX64_MMU_PTB_PFN_MASK     0x0000FFFFFFFFFF00ULL

#endif /* __ARCH_EMEX64_MMU_H */
