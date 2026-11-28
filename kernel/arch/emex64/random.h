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

#ifndef __ARCH_EMEX64_RANDOM_H
#define __ARCH_EMEX64_RANDOM_H

#include <stdint.h>

#define rand64(cr)                                                              \
({                                                                              \
    uint64_t __v;                                                               \
    __asm__ __volatile__("rdrnd %0" : "=r"(__v));                               \
    __v;                                                                        \
})

#endif /* __ARCH_EMEX64_RANDOM_H */
