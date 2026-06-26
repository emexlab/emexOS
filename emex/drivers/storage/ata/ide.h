/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: ide.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef PCI_IDE_H
#define PCI_IDE_H

#include <types.h>

int pci_find_ide_controller(void);
int pci_ide_is_found(void);

u16 pci_ide_get_primary_base(void);
u16 pci_ide_get_primary_ctrl(void);
u16 pci_ide_get_secondary_base(void);
u16 pci_ide_get_secondary_ctrl(void);

#endif
