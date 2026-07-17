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

#include "ahci.h"
#include <kernel/pci/pci.h>
#include <kernel/pci/config.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/mem/phys/physmem.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/mem/lib/main.h>
#include <kernel/include/reqs.h>
#include <kernel/kernel_processes/bootscreen/log.h>
#include <kernel/graph/lib/string.h>
#include <kernel/kernel_processes/tm/cfg/stdclrs.h>

#include <types.h>
#include <drivers/storage/ata/disk.h>

static void* ahci_base = NULL;
extern klime_t *klime;

// Helper to find a free command slot
int ahci_find_cmd_slot(u8* port_base) {
    u32 pi = *(volatile u32*)(port_base + AHCI_PXCI);
    u32 sact = *(volatile u32*)(port_base + AHCI_PXSACT);
    u32 slots = (pi | sact);
    for (int i = 0; i < 32; i++) {
        if ((slots & (1 << i)) == 0) return i;
    }
    return -1;
}

void ahci_port_stop(u8* port_base) {
    volatile u32* cmd = (volatile u32*)(port_base + AHCI_PXCMD);
    *cmd &= ~AHCI_PXCMD_ST;
    *cmd &= ~AHCI_PXCMD_FRE;

    while (1) {
        if (!(*cmd & AHCI_PXCMD_FR) && !(*cmd & AHCI_PXCMD_CR)) break;
    }
}

void ahci_port_start(u8* port_base) {
    volatile u32* cmd = (volatile u32*)(port_base + AHCI_PXCMD);
    while (*cmd & AHCI_PXCMD_CR);
    *cmd |= AHCI_PXCMD_FRE;
    *cmd |= AHCI_PXCMD_ST;
}

int ahci_identify(u8* port_base, u64 buffer_phys) {
    int slot = ahci_find_cmd_slot(port_base);
    if (slot == -1) return 0;

    u32 clb = *(volatile u32*)(port_base + AHCI_PXCLB);
    u32 clbu = *(volatile u32*)(port_base + AHCI_PXCLBU);
    u64 cl_phys = ((u64)clbu << 32) | clb;
    hba_cmd_header_t* cmd_header = (hba_cmd_header_t*)phys_to_virt(hhdm_request.response, cl_phys);
    cmd_header += slot;

    cmd_header->cfl = sizeof(fis_reg_h2d_t) / sizeof(u32);
    cmd_header->w = 0;
    cmd_header->prdtl = 1;

    u64 ct_phys = ((u64)cmd_header->ctbau << 32) | cmd_header->ctba;
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)phys_to_virt(hhdm_request.response, ct_phys);
    memset(cmd_table, 0, sizeof(hba_cmd_table_t));

    cmd_table->prdt_entry[0].dba = (u32)buffer_phys;
    cmd_table->prdt_entry[0].dbau = (u32)(buffer_phys >> 32);
    cmd_table->prdt_entry[0].dbc = 511;
    cmd_table->prdt_entry[0].i = 1;

    fis_reg_h2d_t* cfis = (fis_reg_h2d_t*)(cmd_table->cfis);
    cfis->fis_type = FIS_TYPE_REG_H2D;
    cfis->c = 1;
    cfis->command = 0xEC; // IDENTIFY DEVICE

    int spin = 0;
    while ((*(volatile u32*)(port_base + AHCI_PXTFD) & (0x80 | 0x08)) && spin < 1000000) {
        spin++;
    }
    if (spin == 1000000) return 0;

    *(volatile u32*)(port_base + AHCI_PXCI) = (1 << slot);

    while (1) {
        if ((*(volatile u32*)(port_base + AHCI_PXCI) & (1 << slot)) == 0) break;
        if (*(volatile u32*)(port_base + AHCI_PXIS) & (1 << 30)) return 0;
    }

    return 1;
}

int ahci_read(u8* port_base, u64 startlba, u32 count, u64 buffer_phys) {
    int slot = ahci_find_cmd_slot(port_base);
    if (slot == -1) return 0;

    u32 clb = *(volatile u32*)(port_base + AHCI_PXCLB);
    u32 clbu = *(volatile u32*)(port_base + AHCI_PXCLBU);
    u64 cl_phys = ((u64)clbu << 32) | clb;
    hba_cmd_header_t* cmd_header = (hba_cmd_header_t*)phys_to_virt(hhdm_request.response, cl_phys);
    cmd_header += slot;

    cmd_header->cfl = sizeof(fis_reg_h2d_t) / sizeof(u32);
    cmd_header->w = 0;
    cmd_header->prdtl = 1;

    u64 ct_phys = ((u64)cmd_header->ctbau << 32) | cmd_header->ctba;
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)phys_to_virt(hhdm_request.response, ct_phys);
    memset(cmd_table, 0, sizeof(hba_cmd_table_t));

    cmd_table->prdt_entry[0].dba = (u32)buffer_phys;
    cmd_table->prdt_entry[0].dbau = (u32)(buffer_phys >> 32);
    cmd_table->prdt_entry[0].dbc = (count * 512) - 1;
    cmd_table->prdt_entry[0].i = 1;

    fis_reg_h2d_t* cfis = (fis_reg_h2d_t*)(cmd_table->cfis);
    cfis->fis_type = FIS_TYPE_REG_H2D;
    cfis->c = 1;
    cfis->command = 0x25; // READ DMA EXT

    cfis->lba0 = (u8)startlba;
    cfis->lba1 = (u8)(startlba >> 8);
    cfis->lba2 = (u8)(startlba >> 16);
    cfis->device = 1 << 6;
    cfis->lba3 = (u8)(startlba >> 24);
    cfis->lba4 = (u8)(startlba >> 32);
    cfis->lba5 = (u8)(startlba >> 40);
    cfis->countl = (u8)count;
    cfis->counth = (u8)(count >> 8);

    while ((*(volatile u32*)(port_base + AHCI_PXTFD) & (0x80 | 0x08)));
    *(volatile u32*)(port_base + AHCI_PXCI) = (1 << slot);

    while (1) {
        if ((*(volatile u32*)(port_base + AHCI_PXCI) & (1 << slot)) == 0) break;
        if (*(volatile u32*)(port_base + AHCI_PXIS) & (1 << 30)) return 0;
    }
    return 1;
}

int ahci_write(u8* port_base, u64 startlba, u32 count, u64 buffer_phys) {
    int slot = ahci_find_cmd_slot(port_base);
    if (slot == -1) return 0;

    u32 clb = *(volatile u32*)(port_base + AHCI_PXCLB);
    u32 clbu = *(volatile u32*)(port_base + AHCI_PXCLBU);
    u64 cl_phys = ((u64)clbu << 32) | clb;
    hba_cmd_header_t* cmd_header = (hba_cmd_header_t*)phys_to_virt(hhdm_request.response, cl_phys);
    cmd_header += slot;

    cmd_header->cfl = sizeof(fis_reg_h2d_t) / sizeof(u32);
    cmd_header->w = 1;
    cmd_header->prdtl = 1;

    u64 ct_phys = ((u64)cmd_header->ctbau << 32) | cmd_header->ctba;
    hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)phys_to_virt(hhdm_request.response, ct_phys);
    memset(cmd_table, 0, sizeof(hba_cmd_table_t));

    cmd_table->prdt_entry[0].dba = (u32)buffer_phys;
    cmd_table->prdt_entry[0].dbau = (u32)(buffer_phys >> 32);
    cmd_table->prdt_entry[0].dbc = (count * 512) - 1;
    cmd_table->prdt_entry[0].i = 1;

    fis_reg_h2d_t* cfis = (fis_reg_h2d_t*)(cmd_table->cfis);
    cfis->fis_type = FIS_TYPE_REG_H2D;
    cfis->c = 1;
    cfis->command = 0x35; // WRITE DMA EXT

    cfis->lba0 = (u8)startlba;
    cfis->lba1 = (u8)(startlba >> 8);
    cfis->lba2 = (u8)(startlba >> 16);
    cfis->device = 1 << 6;
    cfis->lba3 = (u8)(startlba >> 24);
    cfis->lba4 = (u8)(startlba >> 32);
    cfis->lba5 = (u8)(startlba >> 40);
    cfis->countl = (u8)count;
    cfis->counth = (u8)(count >> 8);

    while ((*(volatile u32*)(port_base + AHCI_PXTFD) & (0x80 | 0x08)));
    *(volatile u32*)(port_base + AHCI_PXCI) = (1 << slot);

    while (1) {
        if ((*(volatile u32*)(port_base + AHCI_PXCI) & (1 << slot)) == 0) break;
        if (*(volatile u32*)(port_base + AHCI_PXIS) & (1 << 30)) return 0;
    }
    return 1;
}

void ahci_init(void) {
    int dev_count = pci_get_device_count();
    pci_device_t* hba_dev = NULL;

    for (int i = 0; i < dev_count; i++) {
        pci_device_t* dev = pci_get_device(i);
        if (dev->class_code == 0x01 && dev->subclass == 0x06) {
            hba_dev = dev;
            break;
        }
    }

    if (!hba_dev) {
        return;
    }

    log("[AHCI]", "Found controller\n", d);

    u32 pci_cmd = pci_config_read(hba_dev->bus, hba_dev->device, hba_dev->function, PCI_COMMAND);
    pci_cmd |= (1 << 1) | (1 << 2);
    pci_config_write(hba_dev->bus, hba_dev->device, hba_dev->function, PCI_COMMAND, pci_cmd);

    u32 bar5 = pci_config_read(hba_dev->bus, hba_dev->device, hba_dev->function, PCI_BAR5);
    u64 phys_base = bar5 & 0xFFFFFFF0;

    if (phys_base == 0) return;

    u64 virt_base = phys_base + hhdm_request.response->offset;
    map_region(hhdm_request.response, phys_base, virt_base, 4096, KERNEL_FLAGS | PTE_PCD);
    ahci_base = (void*)virt_base;

    volatile u32* ghc = (volatile u32*)((u8*)ahci_base + AHCI_REG_GHC);
    *ghc |= AHCI_GHC_HR;
    while (*ghc & AHCI_GHC_HR);
    *ghc |= AHCI_GHC_AE;

    u32 vs = *(volatile u32*)((u8*)ahci_base + AHCI_REG_VS);
    char vs_buf[32]; vs_buf[0] = '\0';
    str_append_uint(vs_buf, (vs >> 16) & 0xFFFF);
    str_append(vs_buf, ".");
    str_append_uint(vs_buf, vs & 0xFFFF);
    log("[AHCI]", "Version: ", d);
    BOOTUP_PRINT(vs_buf, white());
    BOOTUP_PRINT("\n", white());

    u32 pi = *(volatile u32*)((u8*)ahci_base + AHCI_REG_PI);
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            u8* port_base = (u8*)ahci_base + AHCI_PORT_BASE + (i * AHCI_PORT_SIZE);
            u32 ssts = *(volatile u32*)(port_base + AHCI_PXSSTS);
            u8 det = ssts & 0x0F;
            if (det == 3) {
                ahci_port_stop(port_base);

                u64 port_mem_phys = physmem_alloc_to(1);
                void* port_mem_virt = phys_to_virt(hhdm_request.response, port_mem_phys);
                memset(port_mem_virt, 0, 4096);

                *(volatile u32*)(port_base + AHCI_PXCLB) = (u32)port_mem_phys;
                *(volatile u32*)(port_base + AHCI_PXCLBU) = (u32)(port_mem_phys >> 32);
                *(volatile u32*)(port_base + AHCI_PXFB) = (u32)(port_mem_phys + 1024);
                *(volatile u32*)(port_base + AHCI_PXFBU) = (u32)((port_mem_phys + 1024) >> 32);

                // Allocate pages for command tables (one for each of the 32 slots)
                // Each command table will get its own 4KB page for simplicity and alignment.
                u64 ct_phys_base = physmem_alloc_to(32);
                void* ct_virt_base = phys_to_virt(hhdm_request.response, ct_phys_base);
                memset(ct_virt_base, 0, 32 * 4096);

                hba_cmd_header_t* cmd_header = (hba_cmd_header_t*)port_mem_virt;
                for (int j = 0; j < 32; j++) {
                    u64 ct_phys = ct_phys_base + (j * 4096);
                    cmd_header[j].ctba = (u32)ct_phys;
                    cmd_header[j].ctbau = (u32)(ct_phys >> 32);
                }

                ahci_port_start(port_base);

                u64 identify_phys = physmem_alloc_to(1);
                ATAidentify_t* ident = (ATAidentify_t*)phys_to_virt(hhdm_request.response, identify_phys);
                memset(ident, 0, 512);

                if (ahci_identify(port_base, identify_phys)) {
                    log("[AHCI]", "Port ", d);
                    char p_buf[10]; p_buf[0] = '\0'; str_append_uint(p_buf, i);
                    BOOTUP_PRINT(p_buf, white());
                    BOOTUP_PRINT(": IDENTIFY Success. Model: ", white());

                    char model[41];
                    u16* model_u16 = (u16*)ident->model;
                    for (int k = 0; k < 20; k++) {
                        u16 word = model_u16[k];
                        model[k*2] = (char)(word >> 8);
                        model[k*2 + 1] = (char)(word & 0xFF);
                    }
                    model[40] = '\0';
                    BOOTUP_PRINT(model, white());
                    BOOTUP_PRINT("\n", white());

                    u64 sectors = 0;
                    if (ident->capabilities[1] & (1 << 10)) {
                        sectors = ((u64)ident->lba48_sectors[3] << 48) |
                                  ((u64)ident->lba48_sectors[2] << 32) |
                                  ((u64)ident->lba48_sectors[1] << 16) |
                                  ((u64)ident->lba48_sectors[0]);
                    } else {
                        sectors = ((u32)ident->lba28_sectors[1] << 16) |
                                  ((u32)ident->lba28_sectors[0]);
                    }

                    // Register with generic storage system
                    ATAregister_ahci_device(i, port_base, model, sectors);
                }
            }
        }
    }
}
