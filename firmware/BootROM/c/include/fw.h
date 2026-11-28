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

#ifndef FW_H
#define FW_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define BOOTROM_VERSION_MAJOR   1
#define BOOTROM_VERSION_MINOR   5
#define BOOTROM_VERSION_PATCH   0

#define EMEX64_PAGE_SIZE        0x2000UL
#define EMEX64_MMIO_BASE        0x0020000000000000UL

#define EMEX64_MC_REG_SIZE      (EMEX64_MMIO_BASE + EMEX64_PAGE_SIZE * 5)
#define EMEX64_APIC_REG_VERSION (EMEX64_MMIO_BASE + EMEX64_PAGE_SIZE * 1 + 0x08)

#define EMEX64_IC_BASE          (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 0))     /* Interrupt Controller */
#define EMEX64_APIC_BASE        (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 1))     /* Advanced Programmable Interrupt Controller */
#define EMEX64_TIMER_BASE       (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 2))
#define EMEX64_RTC_BASE         (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 3))
#define EMEX64_UART_BASE        (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 4))
#define EMEX64_MC_BASE          (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 5))
#define EMEX64_PLATFORM_BASE    (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 6))
#define EMEX64_8042_BASE        (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 7))
#define EMEX64_PCIE_BASE        (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 8))     /* PCIe Root Complex */
#define EMEX64_IOAPIC_BASE      (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 9))     /* Input Output Advanced Programmable Interrupt Controller */
#define EMEX64_CPUID_BASE       (EMEX64_MMIO_BASE + (EMEX64_PAGE_SIZE * 10))    /* CPU Identification */

/* PCIe related crap*/
#define EMEX64_PCIE_ECAM_BASE   (EMEX64_MMIO_BASE + 0x0000000100000000)
#define EMEX64_PCIE_ECAM_SIZE   0x0000000020000000
#define EMEX64_PCIE_MMIO_BASE   (EMEX64_MMIO_BASE + 0x0000000200000000)
#define EMEX64_PCIE_MMIO_SIZE   0x0000000100000000

extern u64 gui_fb;
extern u16 gui_width, gui_height;
extern u64 display_bar;

static inline u64 fw_ram_end(void)
{
    return gui_fb ? gui_fb : *(volatile u64 *)EMEX64_MC_REG_SIZE;
}

#define EMEX64_TIMER            (EMEX64_MMIO_BASE + EMEX64_PAGE_SIZE * 2)
#define TIMER_CTRL              0x00
#define TIMER_COUNT             0x08
#define TIMER_COMPARE           0x10
#define TIMER_STATUS            0x18
#define TIMER_FREQ              0x20
#define TIMER_CTRL_ENABLE       (1UL << 0)
#define TIMER_CTRL_IRQ          (1UL << 1)
#define TIMER_CTRL_PERIODIC     (1UL << 2)

#define EMEX64_PLATFORM_REG_PWR (EMEX64_PLATFORM_BASE + 0x00)

#define EMEX64_MC_REG_KTRR_SIZE   (EMEX64_MC_REG_SIZE + 0x08)
#define EMEX64_MC_REG_KTRR_LOCKED (EMEX64_MC_REG_SIZE + 0x10)
#define EMEX64_MC_REG_KTRR_BASE   (EMEX64_MC_REG_SIZE + 0x18)

/* CPUID ECID shit */
#define EMEX64_CPUID_REG_ECID   (EMEX64_CPUID_BASE + 0x20)

/* MMIO registers are aligned to a quad word boundary */
static inline u64 mmio_read64(u64 addr)
{
    return *(volatile u64 *)addr;
}

static inline void mmio_write64(u64 addr,
                                u64 value)
{
    *(volatile u64 *)addr = value;
}

#define FW_CR_CREL      0
#define FW_CR_CRKSP     1
#define FW_CR_CREXC     2
#define FW_CR_CRISA     6

#define fw_cr_read(cr)                                                          \
({                                                                              \
    u64 __v;                                                                    \
    __asm__ __volatile__("cmovb %0, %1" : "=r"(__v) : "i"(cr));                 \
    __v;                                                                        \
})

#define fw_cr_write(cr, val)                                                    \
do                                                                              \
{                                                                               \
    u64 __v = (u64)(val);                                                       \
    __asm__ __volatile__("cmov %0, %1" : : "i"(cr), "r"(__v) : "memory");       \
} while(0)

static inline void fw_hlt(void)
{
    __asm__ __volatile__("hlt" ::: "memory");
}

/* string.c */
void *memset(void *dst, int value, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
void *memmove(void *dst, const void *src, size_t len);

#define fw_memset memset
#define fw_memcpy memcpy

/* put.c */
void fw_putc(int c);
void fw_puts(const char *s);

/* fwlib.c */
void fw_printf(const char *fmt, ...);
void fw_vprintf(const char *fmt, va_list ap);
int fw_memcmp(const void *a, const void *b, u64 len);
u32 fw_crc32_update(u32 crc, const void *data, u64 len);
void fw_put_size(u64 bytes);
void fw_jump(u64 entry, void *arg) __attribute__((noreturn));

/* exc.c */
void fw_hang(void) __attribute__((noreturn));
void panic(const char *why) __attribute__((noreturn));

/* the linker script exposes those */
extern char __data_start[];
extern char __bss_start[];
extern char __bss_end[];

#endif /* FW_H */
