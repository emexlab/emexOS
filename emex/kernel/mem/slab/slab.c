/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: slab.c
 * CREATED BY: tsaraki
 * MODIFIED BY: --
 *
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
