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

#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include "export.h"

void eboot_putc(char c);

EBOOT_EXPORT int printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
EBOOT_EXPORT int vprintf(const char *fmt, va_list ap) __attribute__((format(printf, 1, 0)));

#endif /* PRINTF_H */
