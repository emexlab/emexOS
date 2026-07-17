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

#ifndef TABLE_H
#define TABLE_H

#include <types.h>
#include <kernel/graph/lib/string.h>

// list of available tags
static const char *html_tags[] = {
    "html",
    "head",
    "title",
    "body",
    "h1",
    "h2",
    "h3",
    "p",
    "div",
    "span",
    "a",
    "ul",
    "ol",
    "li",
    "table",
    "tr",
    "td",
    "th",
    "br",
    "hr"
};

static const int html_tags_count = sizeof(html_tags) / sizeof(html_tags[0]);

#endif
