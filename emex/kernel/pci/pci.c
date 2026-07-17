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

#include "pci.h"
#include "device.h"
#include "config.h"
#include "express.h"
#include <kernel/graph/lib/string.h>
#include <kernel/kernel_processes/tm/cfg/stdclrs.h>
#include <kernel/graph/theme.h>

void pci_init(void) {
    char buf[64];
    buf[0] = '\0'; // prevents it from random character drawing
// before pci_get_device_count();
// because in the kernel its used before this causes random character drawing

    log("[PCI]", "Init PCI/PCIe\n", d);

    pci_device_init();


    int count = pci_device_get_count();

    // count PCIe devices
    int pcie_count = 0;
    for (int i = 0; i < count; i++) {
        pci_device_t *dev = pci_device_get(i);
        if (dev && pcie_is_device(dev->bus, dev->device, dev->function)) {
            pcie_count++;
        }
    }
    log("[PCI]", "found ", d);
    str_append_uint(buf, pcie_count);
    BOOTUP_PRINT(buf, white());
    BOOTUP_PRINT(" device(s)\n", white());

}

int pci_get_device_count(void) {
    return pci_device_get_count();
}

pci_device_t* pci_get_device(int index) {
    return pci_device_get(index);
}
