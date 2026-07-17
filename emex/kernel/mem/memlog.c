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

#include "memlog.h"

/*
 * prints the memory map start and hhdm offset
 */
void memlog_print_map(void)
{
    if (memmap_request.response)
    {
        log("[MEM]", "memmap start:\n", d);

        for (size_t i = 0; i < memmap_request.response->entry_count; i++)
        {
            struct limine_memmap_entry *e = memmap_request.response->entries[i];
            char buf[128];
            char tmp[32];
            // convert numbers to hex manually
            str_copy(buf, "Type: ");
            str_append_uint(buf, e->type);
            str_append(buf, ", Base: 0x");
            str_from_hex(tmp, e->base);
            str_append(buf, tmp);
            str_append(buf, ", Length: 0x");
            str_from_hex(tmp, e->length);
            str_append(buf, tmp);
            str_append(buf, "\n");
            log("[MEM]", buf, d);
        }

        log("[MEM]", ";\n", d);
    } else
    {
        log("[MEM]", "NOTHING FOUND\n", error);
    }

    if (hhdm_request.response)
    {
        char buf[64];
        char tmp[32];
        str_copy(buf, "HHDM Offset: 0x");
        str_from_hex(tmp, hhdm_request.response->offset);
        str_append(buf, tmp);
        str_append(buf, "\n");
        log("[MEM]", buf, d);
    }
}
