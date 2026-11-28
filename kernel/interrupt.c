/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
 */

#include "interrupt.h"
#include <stddef.h>

#define KERNEL_PAGE_SIZE        0x2000UL
#define KERNEL_MMIO_BASE        0x0020000000000000UL
#define KERNEL_IC_BASE          (KERNEL_MMIO_BASE + 0)
#define KERNEL_IC_REG_PENDING   (KERNEL_IC_BASE + 0x00)
#define KERNEL_IC_REG_ENABLED   (KERNEL_IC_BASE + 0x08)
#define KERNEL_IC_REG_CTRL      (KERNEL_IC_BASE + 0x10)
#define KERNEL_IC_REG_VECTOR    (KERNEL_IC_BASE + 0x18)
#define KERNEL_IC_REG_ACK       (KERNEL_IC_BASE + 0x20)
#define KERNEL_IC_REG_CURRENT   (KERNEL_IC_BASE + 0x28)
#define KERNEL_IC_CTRL_ENABLE   0x1

#define KERNEL_CR_CRKSP         1
#define ICM_STACK_SIZE          KERNEL_PAGE_SIZE

static uint64_t icm_vector[KERNEL_IC_IRQS];
static icm_handler_t icm_handlers[KERNEL_IC_IRQS];
static uint8_t icm_stack[ICM_STACK_SIZE] __attribute__((aligned(16)));
static int icm_ready;

static int icm_irq_is_valid(uint64_t irq)
{
    return irq < KERNEL_IC_IRQS;
}

static uint64_t icm_irq_mask(uint64_t irq)
{
    return 1UL << irq;
}

static inline uint64_t icm_mmio_read(uint64_t addr)
{
    return *(volatile uint64_t *)addr;
}

static inline void icm_mmio_write(uint64_t addr, uint64_t value)
{
    *(volatile uint64_t *)addr = value;
}

#define icm_cr_write(cr, value)                                                \
do                                                                              \
{                                                                               \
    uint64_t __value = (uint64_t)(value);                                       \
    __asm__ __volatile__("cmov %0, %1" : : "i"(cr), "r"(__value) : "memory");  \
} while(0)

void icm_dispatch(struct icm_frame *frame);

__attribute__((naked)) static void icm_trap_entry(void)
{
    __asm__ __volatile__(
        "push rr, er0, er1, er2, er3, er4, er5, er6, er7\n\t"
        "mov r0, fp\n\t"
        "bl icm_dispatch\n\t"
        "pop er7, er6, er5, er4, er3, er2, er1, er0, rr\n\t"
        "iret");
}

void icm_dispatch(struct icm_frame *frame)
{
    uint64_t irq = icm_mmio_read(KERNEL_IC_REG_CURRENT);

    if(irq < KERNEL_IC_IRQS)
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
    icm_mmio_write(KERNEL_IC_REG_ENABLED, 0);

    icm_cr_write(KERNEL_CR_CRKSP,
                 (uint64_t)icm_stack + ICM_STACK_SIZE - 8);

    for(uint64_t irq = 0; irq < KERNEL_IC_IRQS; irq++)
    {
        icm_vector[irq] = (uint64_t)icm_trap_entry;
        icm_handlers[irq] = NULL;
    }

    icm_mmio_write(KERNEL_IC_REG_PENDING, 0);
    icm_mmio_write(KERNEL_IC_REG_VECTOR, (uint64_t)icm_vector);
    icm_mmio_write(KERNEL_IC_REG_CTRL, KERNEL_IC_CTRL_ENABLE);
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
    for(uint64_t irq = 0; irq < KERNEL_IC_IRQS; irq++)
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

    uint64_t enabled = icm_mmio_read(KERNEL_IC_REG_ENABLED);
    icm_mmio_write(KERNEL_IC_REG_ENABLED, enabled | icm_irq_mask(irq));
}

void icm_disable_irq(uint64_t irq)
{
    if(!icm_ready || !icm_irq_is_valid(irq))
    {
        return;
    }

    uint64_t enabled = icm_mmio_read(KERNEL_IC_REG_ENABLED);
    icm_mmio_write(KERNEL_IC_REG_ENABLED, enabled & ~icm_irq_mask(irq));
}

void icm_ack_irq(uint64_t irq)
{
    if(icm_ready && icm_irq_is_valid(irq))
    {
        icm_mmio_write(KERNEL_IC_REG_ACK, irq);
    }
}
