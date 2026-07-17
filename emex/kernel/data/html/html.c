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

#include "html.h"

html_entry_t html_table[128];
int html_table_count = 0;

static void skipWS(char **p) {
    while (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n') (*p)++;
}

void htmlparse(char *buf) {
    char *p = buf;
    skipWS(&p);
    html_table_count = 0;

    while (*p) {
        if (*p == '<') {
            p++;
            if (*p == '/') {
                while (*p && *p != '>') p++;
                if (*p) p++;
                skipWS(&p);
                continue;
            }

            char *tag_start = p;
            while (*p && *p != '>' && *p != ' ' && *p != '\n') p++;
            char saved = *p;
            *p = '\0';
            char *tag_name = tag_start;
            *p = saved;
            while (*p && *p != '>') p++;
            if (*p) p++;

            char *content_start = p;
            while (*p && !(*p == '<' && *(p+1) == '/')) p++;
            char *content_end = p;
            if (content_end > content_start) *content_end = '\0';

            html_table[html_table_count].tag = tag_name;
            html_table[html_table_count].content = content_start;
            html_table_count++;

            if (*p == '<') {
                p += 2;
                while (*p && *p != '>') p++;
                if (*p) p++;
            }

            skipWS(&p);
        } else {
            p++;
        }
    }
}

char* htmlget(const char *tag) {
    for (int i = 0; i < html_table_count; i++) {
        if (str_equals(html_table[i].tag, tag)) return html_table[i].content;
    }
    return 0;
}
