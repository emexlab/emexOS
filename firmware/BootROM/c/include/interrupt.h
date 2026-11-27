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

#ifndef FW_INTERRUPT_H
#define FW_INTERRUPT_H

#include <fw.h>

#define EMEX64_IC_REG_PENDING   (EMEX64_IC_BASE + 0x00)
#define EMEX64_IC_REG_ENABLED   (EMEX64_IC_BASE + 0x08)
#define EMEX64_IC_REG_CTRL      (EMEX64_IC_BASE + 0x10)
#define EMEX64_IC_REG_VECTOR    (EMEX64_IC_BASE + 0x18)
#define EMEX64_IC_REG_ACK       (EMEX64_IC_BASE + 0x20)
#define EMEX64_IC_REG_CURRENT   (EMEX64_IC_BASE + 0x28)

#define EMEX64_IC_CTRL_ENABLE   0x1

#define EMEX64_IC_IRQ_EXCEPTION 0
#define EMEX64_IC_IRQ_TIMER     1
#define EMEX64_IC_IRQ_DISK      2
#define EMEX64_IC_IRQ_NETWORK   3
#define EMEX64_IC_IRQ_SOFTWARE  4
#define EMEX64_IC_IRQ_UART      5
#define EMEX64_IC_IRQ_8042      6

#define EMEX64_IC_IRQS          64

/*
 * what the CPU pushed on crksp before it entered the handler (pc to r9,
 * fp points below r9) and what interrupt_entry pushed below that (rr
 * first, at fp). iret takes the CPU's part back, so a handler can change
 * where it returns to through pc.
 */
struct fw_frame {
    u64 rr;
    u64 r9, r8, r7, r6, r5, r4, r3, r2, r1, r0;
    u64 fpc, cf, fp, sp, pc, level;
};

typedef void (*interrupt_handler_t)(struct fw_frame *frame);

void interrupt_init(void);
void interrupt_register_handler(u64 irq, interrupt_handler_t handler);
void interrupt_unregister_handler(u64 irq);
void interrupt_unregister_all(void);

void exception_handler(struct fw_frame *frame);
void kbd_handler(struct fw_frame *frame);

#endif /* FW_INTERRUPT_H */
