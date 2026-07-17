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

#ifndef PCI_EXPRESS_H
#define PCI_EXPRESS_H

#include <types.h>

typedef enum {
    PCIE_SPEED_UNKNOWN = 0,
    PCIE_SPEED_2_5_GT = 1,   // gen 1
    PCIE_SPEED_5_GT = 2,     // gen 2
    PCIE_SPEED_8_GT = 3,     // gen 3
    PCIE_SPEED_16_GT = 4,    // gen 4
    PCIE_SPEED_32_GT = 5     // gen 5
} pcie_link_speed_t;

// PCIe detection and info functions
int pcie_is_device(u8 bus, u8 device, u8 function);
pcie_link_speed_t pcie_get_link_speed(u8 bus, u8 device, u8 function);
u8 pcie_get_link_width(u8 bus, u8 device, u8 function);
const char* pcie_get_speed_string(pcie_link_speed_t speed);

#endif
