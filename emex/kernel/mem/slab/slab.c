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

#include "slab.h"
#include <kernel/communication/serial.h>
#include <kernel/arch/x86_64/exceptions/panic.h>
#include <kernel/mem/lib/main.h>

void slab_init(slab_allocator_t *ptr_slab, u64 *ptr_slab_meta, u64 *ptr_slab_data) {
  if (!ptr_slab) {
      panic("\nInvalid ptr slab\n");
  }

  if (!ptr_slab_meta) {
      panic("\nInvalid ptr slab meta\n");
  }

  if (!ptr_slab_data) {
      panic("\nInvalid ptr slab data\n");
  }

  memset(ptr_slab, 0, sizeof(slab_allocator_t));

}
