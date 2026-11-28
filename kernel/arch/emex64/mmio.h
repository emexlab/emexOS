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

#ifndef __ARCH_EMEX64_MMIO_H
#define __ARCH_EMEX64_MMIO_H

#include <stdint.h>
#include <arch/emex64/page.h>

#define EMEX64_MMIO_BASE            0x0020000000000000

/* MMIO base addresses */
#define EMEX64_IC_BASE              (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 0))     /* Interrupt Controller */
#define EMEX64_APIC_BASE            (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 1))     /* Advanced Programmable Interrupt Controller */
#define EMEX64_TIMER_BASE           (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 2))
#define EMEX64_RTC_BASE             (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 3))     /* Real Time Clock */
#define EMEX64_UART_BASE            (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 4))     /* Universal Asynchronous Receiver Transmitter */
#define EMEX64_MC_BASE              (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 5))     /* Memory Controller */
#define EMEX64_PLATFORM_BASE        (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 6))
#define EMEX64_8042_BASE            (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 7))
#define EMEX64_PCIE_BASE            (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 8))     /* Peripheral Component Interconnect Express Root Complex */
#define EMEX64_IOAPIC_BASE          (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 9))     /* Input Output Advanced Programmable Interrupt Controller */
#define EMEX64_CPUID_BASE           (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 10))    /* Central Processing Unit Identification */
#define EMEX64_NVRAM_BASE           (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 11))    /* Non Volatile Random Access Memory */

/* MMIO sizes */
#define EMEX64_INTC_SIZE            0x30
#define EMEX64_APIC_SIZE            0x80
#define EMEX64_TIMER_SIZE           0x28
#define EMEX64_RTC_SIZE             0x38
#define EMEX64_UART_SIZE            0x18
#define EMEX64_MC_SIZE              0x20
#define EMEX64_PLATFORM_SIZE        0x08
#define EMEX64_8042_SIZE            0x10
#define EMEX64_PCIE_SIZE            0x40
#define EMEX64_IOAPIC_SIZE          0x300
#define EMEX64_CPUID_SIZE           0x80
#define EMEX64_NVRAM_SIZE           0x10008

/* PCIe related crap */
#define EMEX64_PCIE_ECAM_BASE       (EMEX64_MMIO_BASE + 0x0000000100000000)
#define EMEX64_PCIE_ECAM_SIZE       0x0000000020000000
#define EMEX64_PCIE_PCI_ECAM_BASE   (EMEX64_MMIO_BASE + 0x0000000140000000)
#define EMEX64_PCIE_PCI_ECAM_SIZE   0x0000000000100000
#define EMEX64_PCIE_MMIO_BASE       (EMEX64_MMIO_BASE + 0x0000000200000000)
#define EMEX64_PCIE_MMIO_SIZE       0x0000000100000000
#define EMEX64_MMIO_END             (EMEX64_PCIE_MMIO_BASE + EMEX64_PCIE_MMIO_SIZE)

/* Interrupt Controller */
#define EMEX64_INTC_REG_PENDING     EMEX64_IC_BASE
#define EMEX64_INTC_REG_ENABLED     (EMEX64_IC_BASE + 0x08)
#define EMEX64_INTC_REG_CTRL        (EMEX64_IC_BASE + 0x10)
#define EMEX64_INTC_REG_VECTOR      (EMEX64_IC_BASE + 0x18)
#define EMEX64_INTC_REG_ACK         (EMEX64_IC_BASE + 0x20)
#define EMEX64_INTC_REG_CURRENT     (EMEX64_IC_BASE + 0x28)

/* Advanced Programmable Interrupt Controller */
#define EMEX64_APIC_REG_ID          EMEX64_APIC_BASE
#define EMEX64_APIC_REG_VERSION     (EMEX64_APIC_BASE + 0x08)
#define EMEX64_APIC_REG_IPI_STATUS  (EMEX64_APIC_BASE + 0x10)
#define EMEX64_APIC_REG_ICR         (EMEX64_APIC_BASE + 0x18)
#define EMEX64_APIC_REG_START       (EMEX64_APIC_BASE + 0x20)
#define EMEX64_APIC_REG_ARG         (EMEX64_APIC_BASE + 0x28)
#define EMEX64_APIC_REG_RUNNING     (EMEX64_APIC_BASE + 0x30)
#define EMEX64_APIC_REG_TIMER       (EMEX64_APIC_BASE + 0x40)

/* Timer */
#define EMEX64_TIMER_REG_CTRL       EMEX64_TIMER_BASE
#define EMEX64_TIMER_REG_COUNT      (EMEX64_TIMER_BASE + 0x08)
#define EMEX64_TIMER_REG_COMPARE    (EMEX64_TIMER_BASE + 0x10)
#define EMEX64_TIMER_REG_STATUS     (EMEX64_TIMER_BASE + 0x18)
#define EMEX64_TIMER_REG_FREQ       (EMEX64_TIMER_BASE + 0x20)

/* Real Time Clock */
#define EMEX64_RTC_REG_SECONDS      EMEX64_RTC_BASE
#define EMEX64_RTC_REG_MINUTES      (EMEX64_RTC_BASE + 0x08)
#define EMEX64_RTC_REG_HOURS        (EMEX64_RTC_BASE + 0x10)
#define EMEX64_RTC_REG_DAY          (EMEX64_RTC_BASE + 0x18)
#define EMEX64_RTC_REG_MONTH        (EMEX64_RTC_BASE + 0x20)
#define EMEX64_RTC_REG_YEAR         (EMEX64_RTC_BASE + 0x28)
#define EMEX64_RTC_REG_WEEKDAY      (EMEX64_RTC_BASE + 0x30)

/* Universal Asynchronous Receiver Transmitter */
#define EMEX64_UART_REG_DATA        EMEX64_UART_BASE
#define EMEX64_UART_REG_STATUS      (EMEX64_UART_BASE + 0x08)
#define EMEX64_UART_REG_CONTROL     (EMEX64_UART_BASE + 0x10)

/* Memory Controller */
#define EMEX64_MC_REG_SIZE          EMEX64_MC_BASE
#define EMEX64_MC_REG_KTRR_SIZE     (EMEX64_MC_BASE + 0x08)
#define EMEX64_MC_REG_KTRR_LOCKED   (EMEX64_MC_BASE + 0x10)
#define EMEX64_MC_REG_KTRR_BASE     (EMEX64_MC_BASE + 0x18)

/* Platform */
#define EMEX64_PLATFORM_REG_PWR     EMEX64_PLATFORM_BASE

/* 8042 controller */
#define EMEX64_8042_REG_DATA        EMEX64_8042_BASE
#define EMEX64_8042_REG_STATUS      (EMEX64_8042_BASE + 0x08)

/* Peripheral Component Interconnect Express Root Complex */
#define EMEX64_PCIE_RC_REG_ID               EMEX64_PCIE_BASE
#define EMEX64_PCIE_RC_REG_ECAM_BASE        (EMEX64_PCIE_BASE + 0x08)
#define EMEX64_PCIE_RC_REG_ECAM_BUSES       (EMEX64_PCIE_BASE + 0x10)
#define EMEX64_PCIE_RC_REG_MMIO_BASE        (EMEX64_PCIE_BASE + 0x18)
#define EMEX64_PCIE_RC_REG_MMIO_SIZE        (EMEX64_PCIE_BASE + 0x20)
#define EMEX64_PCIE_RC_REG_IRQ_BASE         (EMEX64_PCIE_BASE + 0x28)
#define EMEX64_PCIE_RC_REG_IRQ_COUNT        (EMEX64_PCIE_BASE + 0x30)
#define EMEX64_PCIE_RC_REG_PCI_ECAM_BASE    (EMEX64_PCIE_BASE + 0x38)

/* Input Output Advanced Programmable Interrupt Controller */
#define EMEX64_IOAPIC_REG_VERSION           EMEX64_IOAPIC_BASE
#define EMEX64_IOAPIC_REG_ROUTE             (EMEX64_IOAPIC_BASE + 0x100)

/* Central Processing Unit Identification */
#define EMEX64_CPUID_REG_VENDOR         EMEX64_CPUID_BASE
#define EMEX64_CPUID_REG_MODEL          (EMEX64_CPUID_BASE + 0x08)
#define EMEX64_CPUID_REG_ISA            (EMEX64_CPUID_BASE + 0x10)
#define EMEX64_CPUID_REG_FEATURES       (EMEX64_CPUID_BASE + 0x18)
#define EMEX64_CPUID_REG_ECID           (EMEX64_CPUID_BASE + 0x20)
#define EMEX64_CPUID_REG_BRAND          (EMEX64_CPUID_BASE + 0x40)

/* asm helpers */
static inline uint64_t mmio_read64(uint64_t addr)
{
    return *(volatile uint64_t *)addr;
}

static inline void mmio_write64(uint64_t addr,
                                uint64_t value)
{
    *(volatile uint64_t *)addr = value;
}

#endif /* __ARCH_EMEX64_MMIO_H */
