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

#include <icm.h>
#include <arch/emex64/mmio.h>
#include <stddef.h>

#define EMEX64_IC_CTRL_ENABLE   0x1

static uint64_t icm_vector[EMEX64_IC_IRQS];
static icm_handler_t icm_handlers[EMEX64_IC_IRQS];
static int icm_ready;

static int icm_irq_is_valid(uint64_t irq)
{
    return irq < EMEX64_IC_IRQS;
}

static uint64_t icm_irq_mask(uint64_t irq)
{
    return 1UL << irq;
}

void icm_dispatch(struct icm_frame *frame);

__attribute__((naked)) static void icm_trap_entry(void)
{
    __asm__ __volatile__(
        "push rr, er0, er1, er2, er3, er4, er5, er6, er7\n\t"
        "mov r0, fp\n\t"
        "bl icm_dispatch@plt\n\t"
        "pop er7, er6, er5, er4, er3, er2, er1, er0, rr\n\t"
        "iret");
}

void icm_dispatch(struct icm_frame *frame)
{
    uint64_t irq = mmio_read64(EMEX64_INTC_REG_CURRENT);

    if(irq < EMEX64_IC_IRQS)
    {
        icm_handler_t handler = icm_handlers[irq];
        if(handler != NULL)
        {
            handler(frame);
        }
        icm_ack_irq(irq);
    }
}

void icm_init(void)
{
    icm_ready = 0;
    mmio_write64(EMEX64_INTC_REG_ENABLED, 0);

    for(uint64_t irq = 0; irq < EMEX64_IC_IRQS; irq++)
    {
        icm_vector[irq] = (uint64_t)icm_trap_entry;
        icm_handlers[irq] = NULL;
    }

    mmio_write64(EMEX64_INTC_REG_PENDING, 0);
    mmio_write64(EMEX64_INTC_REG_VECTOR, (uint64_t)icm_vector);
    mmio_write64(EMEX64_INTC_REG_CTRL, EMEX64_IC_CTRL_ENABLE);
    icm_ready = 1;
}

int icm_register_handler(uint64_t irq, icm_handler_t handler)
{
    if(!icm_ready || !icm_irq_is_valid(irq) || handler == NULL)
    {
        return -1;
    }

    icm_handlers[irq] = handler;
    icm_enable_irq(irq);
    return 0;
}

int icm_unregister_handler(uint64_t irq)
{
    if(!icm_ready || !icm_irq_is_valid(irq))
    {
        return -1;
    }

    icm_disable_irq(irq);
    icm_ack_irq(irq);
    icm_handlers[irq] = NULL;
    return 0;
}

void icm_unregister_all(void)
{
    for(uint64_t irq = 0; irq < EMEX64_IC_IRQS; irq++)
    {
        icm_unregister_handler(irq);
    }
}

void icm_enable_irq(uint64_t irq)
{
    if(!icm_ready || !icm_irq_is_valid(irq))
    {
        return;
    }

    uint64_t enabled = mmio_read64(EMEX64_INTC_REG_ENABLED);
    mmio_write64(EMEX64_INTC_REG_ENABLED, enabled | icm_irq_mask(irq));
}

void icm_disable_irq(uint64_t irq)
{
    if(!icm_ready || !icm_irq_is_valid(irq))
    {
        return;
    }

    uint64_t enabled = mmio_read64(EMEX64_INTC_REG_ENABLED);
    mmio_write64(EMEX64_INTC_REG_ENABLED, enabled & ~icm_irq_mask(irq));
}

void icm_ack_irq(uint64_t irq)
{
    if(icm_ready && icm_irq_is_valid(irq))
    {
        mmio_write64(EMEX64_INTC_REG_ACK, irq);
    }
}
