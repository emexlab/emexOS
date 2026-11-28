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

#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

#include <stdint.h>

#define EMEX64_IC_IRQS          64
#define EMEX64_IC_IRQ_EXCEPTION 0
#define EMEX64_IC_IRQ_TIMER     1
#define EMEX64_IC_IRQ_DISK      2
#define EMEX64_IC_IRQ_NETWORK   3
#define EMEX64_IC_IRQ_SOFTWARE  4
#define EMEX64_IC_IRQ_UART      5
#define EMEX64_IC_IRQ_8042      6

struct icm_frame {
    uint64_t rr;
    uint64_t r9, r8, r7, r6, r5, r4, r3, r2, r1, r0;
    uint64_t fpc, cf, fp, sp, pc, level;
};

typedef void (*icm_handler_t)(struct icm_frame *frame);

void icm_init(void);
int icm_register_handler(uint64_t irq, icm_handler_t handler);
int icm_unregister_handler(uint64_t irq);
void icm_unregister_all(void);
void icm_enable_irq(uint64_t irq);
void icm_disable_irq(uint64_t irq);
void icm_ack_irq(uint64_t irq);

#endif
