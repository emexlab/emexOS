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

#include <fw.h>
#include <uart.h>

#define EMEX64_UART_REG_DATA    (EMEX64_UART_BASE + 0x00)
#define EMEX64_UART_REG_STATUS  (EMEX64_UART_BASE + 0x08)
#define EMEX64_UART_REG_CONTROL (EMEX64_UART_BASE + 0x10)

#define EMEX64_UART_STATUS_RX_READY 0x01
#define EMEX64_UART_STATUS_TX_EMPTY 0x02

void uart_putc(const char c)
{
    while((mmio_read64(EMEX64_UART_REG_STATUS) & EMEX64_UART_STATUS_TX_EMPTY) == 0) {};
    mmio_write64(EMEX64_UART_REG_DATA, (u64)c);
}

char uart_getc(void)
{
    while((mmio_read64(EMEX64_UART_REG_STATUS) & EMEX64_UART_STATUS_RX_READY) == 0) {};
    return (char)mmio_read64(EMEX64_UART_REG_DATA);
}
