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

#include "rtl8139.h"
#include <kernel/pci/pci.h>
#include <kernel/pci/config.h>
#include <kernel/mem/phys/physmem.h>
#include <kernel/mem/lib/main.h>
#include <kernel/graph/lib/string.h>
#include <kernel/include/ports.h>
/*state */
static struct {
    u8 mac[6];
    int present;
} dev;

/* initialize RTL8139 device
 *
 * RTL8139 uses I/O ports instead of MMIO
 */
int rtl8139_init(void)
{
    memset(&dev, 0, sizeof(dev));

    /* try to find RTL8139 PCI device */
    pci_device_t *pci = pci_device_find_by_vendor(0x10EC, 0x8139);

    if (!pci)
        return -1; /* -1 on failure */

    /* RTL8139 uses I/O ports instead of MMIO*/
    u16 cmd = pci_config_read_word(pci->bus, pci->device, pci->function, 0x04);
    cmd |= (1 << 0) | (1 << 2); /* activate 0 (io space)
    2 (bus master) */

    pci_config_write_word(pci->bus, pci->device, pci->function, 0x04, cmd);

    /* note:
     * i didnt finish rtl8139 because i just wanted to get networking just working with the e1000 for now
     */
    u32 io_base = pci_config_read(pci->bus, pci->device, pci->function, 0x10) & ~0x3;

    for (int i = 0; i < 6; i++)
    {
        dev.mac[i] = inb(io_base + i);
    }

    dev.present = 1;

    return 0;
}

/*
 * Send packete
 */
int rtl8139_send(const void *data, u16 len)
{
    if (
    	!dev.present || !data || !len
    )return -1;

    /* like i said... not finished
     * TODO: TX logic */
    return -1;
}
int rtl8139_recv(void *buf, u16 max_len)
{
    if (!dev.present || !buf)
        return 0;

    /* TODO: RX logic */
    return 0;
}

void rtl8139_get_mac(u8 mac[6])
{
    for (int i = 0; i < 6; i++)mac[i] = dev.mac[i];
}

/* network driver layer */
/*static net_driver_t rtl_driver =
{
    .init 		= rtl8139_init,
    .send 		= rtl8139_send,
    .recv 		= rtl8139_recv,
    .get_mac 	= rtl8139_get_mac
};*/
