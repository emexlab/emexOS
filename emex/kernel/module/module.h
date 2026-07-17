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

#ifndef DRIVER_MODULE_H
#define DRIVER_MODULE_H

#include <types.h>

#define MAX_MODULES 256
#define VERSION_NUM(major, minor, patch, build) \
  ((major << 24) | (minor << 16) | (patch << 8) | build)

typedef struct driver_module {
  const char *name;
  const char *mount; // mount point like /dev/console
  u32 version;       // actually not neccesary but cool

  // init/cleanup
  int (*init)(void);
  void (*fini)(void);

  // not used just for the file system in future
  //
  void *(*open)(const char *path);
  int (*read)(void *handle, void *buf, size_t count, u64 offset);
  int (*write)(void *handle, const void *buf, size_t count, u64 offset);

} driver_module;

void module_init(void);
int module_register(driver_module *module);
void module_unregister(const char *name);
driver_module *module_find(const char *name);
int module_get_count(void);
driver_module *module_get_by_index(int idx);

#endif
