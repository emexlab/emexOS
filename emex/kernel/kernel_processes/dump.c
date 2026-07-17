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

#include "loader.h"

#include <kernel/communication/serial.h>
#include <kernel/graph/lib/string.h>
#include <kernel/kernel_processes/bootscreen/print.h>

// only reason that this is in a extra file is cuz i want to make it more advanced... but not yet...

/*
void dump_kprocesses(void) {
    printf("[KPROC] table (%d/%d)\n", kproc_cnt, KPROC_MAX);
    for (int i = 0; i < kproc_cnt; i++) {
        kproc_t *p = kproc_table[i];
        if (!p) continue;
        printf("  [%d] %-20s  (%-8s)  (%d)  flags=0x%02X\n",
               i, p->name, state_str(p->state), p->priority, p->flags);
    }
}
*/

void dump_kprocesses(void) {
    print("kernel processes table (", white());
    printInt(kproc_cnt, white());
    print("/", white());
    printInt(KPROC_MAX, white());
    print(")\n", white());

    for (int i = 0; i < kproc_cnt; i++) {
        kproc_t *p = kproc_table[i];
        if (!p) continue;

        print("  [", white());
        printInt(i, white());
        print("] ", white());

        print(p->name, white());

        print("  (", white());
        print(state_str(p->state), white());
        print(")  (", white());

        printInt(p->priority, white());

        print(")  flags=0x", white());

        char hex[32];
        str_from_hex(hex, p->flags);
        print(hex, white());

        print("\n", white());
    }
}
