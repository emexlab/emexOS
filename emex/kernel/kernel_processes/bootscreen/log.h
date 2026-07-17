/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * emexOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * emexOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with emexOS. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LOG_H
#define LOG_H

#include <types.h>
#include <kernel/graph/theme.h>

// Log levels
typedef enum {
    LDEF = 0,
    LSUCCESS,
    LWARNING,
    LERROR,
    LDEBUG_TAG
} log_level_t;

// Keywords for log levels
#define d       LDEF
#define _d      LDEBUG_TAG
#define success LSUCCESS
#define warning LWARNING
#define error   LERROR

// Log colors based on level
#define LCOLOR_TAG     green()//GFX_GRAY_70
#define LCOLOR_BRACKET GFX_GRAY_70
#define LCOLOR_D       white()
#define LCOLOR_SCS     white()//green()
#define LCOLOR_WAR     white()//yellow()
#define LCOLOR_ERR     red()
// optional macro
#define log(tag, message, ...) log_message(tag, message, _LOG_LEVEL(__VA_ARGS__))
#define _LOG_LEVEL(...) _LOG_LEVEL_IMPL(__VA_ARGS__, LDEF)
#define _LOG_LEVEL_IMPL(level, ...) level

void log_message(const char *tag, const char *message, log_level_t level);

//alias logf
void log_printf(log_level_t level, const char *tag, const char *format, ...);

#define logf(level, tag, format, ...) log_printf(level, tag, format, ##__VA_ARGS__)

#endif
