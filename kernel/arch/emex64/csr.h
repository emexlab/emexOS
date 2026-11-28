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

#ifndef __ARCH_EMEX64_CSR_H
#define __ARCH_EMEX64_CSR_H

#include <stdint.h>

#define EMEX64_CREG_EL          0
#define EMEX64_CREG_KSP         1
#define EMEX64_CREG_EXEC        2
#define EMEX64_CREG_VEC         3
#define EMEX64_CREG_PTB         4
#define EMEX64_CREG_FPC         5
#define EMEX64_CREG_ISA         6
#define EMEX64_CREG_STAT        7
#define EMEX64_CREG_TVEC        8
#define EMEX64_CREG_EPC         9
#define EMEX64_CREG_CAUSE       10
#define EMEX64_CREG_TVAL        11
#define EMEX64_CREG_SCRATCH     12
#define EMEX64_CREG_IAKEY       13
#define EMEX64_CREG_DAKEY       14
#define EMEX64_CREG_PAC         15

#define cr_read(cr)                                                             \
({                                                                              \
    uint64_t __v;                                                               \
    __asm__ __volatile__("cmovb %0, %1" : "=r"(__v) : "i"(cr));                 \
    __v;                                                                        \
})

#define cr_write(cr, val)                                                       \
do                                                                              \
{                                                                               \
    uint64_t __v = (uint64_t)(val);                                             \
    __asm__ __volatile__("cmov %c0, %1" : : "i"(cr), "r"(__v) : "memory");      \
} while (0)

#endif /* __ARCH_EMEX64_CSR_H */