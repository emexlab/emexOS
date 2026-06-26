/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: dump.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
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