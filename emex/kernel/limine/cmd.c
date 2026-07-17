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

#include <kernel/include/reqs.h>
#include <kernel/graph/lib/string.h>

static char token[64] = {0};

void cmd_init(void) {
    if (!cmdline_request.response) return;
    const char *cl = cmdline_request.response->cmdline;
    if (!cl) return;

    int i = 0;
    while (cl[i] && i < 63) { token[i] = cl[i]; i++; }
    token[i] = '\0';
    printf("[CMD] cmdline: '%s'\n", token);
}

int cmd_is(const char *val){return str_equals(token,val);}

int cmd_has(const char *t){return str_contains(token,t);}
