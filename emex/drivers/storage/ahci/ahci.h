/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: ahci.h
 * CREATED BY: offihito
 * MODIFIED BY: --
 *
 */
#ifndef AHCI_H
#define AHCI_H

#include <types.h>
#include <kernel/pci/device.h>

// AHCI PCI Configuration
#define AHCI_PCI_BAR5 0x24

// HBA Generic Host Control Registers
#define AHCI_REG_CAP      0x00   // Host Capabilities
#define AHCI_REG_GHC      0x04   // Global Host Control
#define AHCI_REG_IS       0x08   // Interrupt Status
#define AHCI_REG_PI       0x0C   // Ports Implemented
#define AHCI_REG_VS       0x10   // Version
#define AHCI_REG_CCC_CTL  0x14   // Command Completion Coalescing Control
#define AHCI_REG_CCC_PTS  0x18   // Command Completion Coalescing Ports
#define AHCI_REG_EM_LOC   0x1C   // Enclosure Management Location
#define AHCI_REG_EM_CTL   0x20   // Enclosure Management Control
#define AHCI_REG_CAP2     0x24   // Host Capabilities Extended
#define AHCI_REG_BOHC     0x28   // BIOS/OS Handoff Control and Status

// GHC bits
#define AHCI_GHC_AE       (1u << 31) // AHCI Enable
#define AHCI_GHC_IE       (1u << 1)  // Interrupt Enable
#define AHCI_GHC_HR       (1u << 0)  // HBA Reset

// Port Register Offsets
#define AHCI_PORT_BASE    0x100
#define AHCI_PORT_SIZE    0x80

#define AHCI_PXCLB        0x00   // Command List Base Address
#define AHCI_PXCLBU       0x04   // Command List Base Address Upper
#define AHCI_PXFB         0x08   // FIS Base Address
#define AHCI_PXFBU        0x0C   // FIS Base Address Upper
#define AHCI_PXIS         0x10   // Interrupt Status
#define AHCI_PXIE         0x14   // Interrupt Enable
#define AHCI_PXCMD        0x18   // Command and Status
#define AHCI_PXTFD        0x20   // Task File Data
#define AHCI_PXSIG        0x24   // Signature
#define AHCI_PXSSTS       0x28   // Serial ATA Status
#define AHCI_PXSCTL       0x2C   // Serial ATA Control
#define AHCI_PXSERR       0x30   // Serial ATA Error
#define AHCI_PXSACT       0x34   // Serial ATA Active
#define AHCI_PXCI         0x38   // Command Issue

// PxCMD bits
#define AHCI_PXCMD_ST     (1u << 0)   // Start
#define AHCI_PXCMD_SUD    (1u << 1)   // Spin Up Device
#define AHCI_PXCMD_POD    (1u << 2)   // Power On Device
#define AHCI_PXCMD_FRE    (1u << 4)   // FIS Receive Enable
#define AHCI_PXCMD_FR     (1u << 14)  // FIS Receive Running
#define AHCI_PXCMD_CR     (1u << 15)  // Command List Running

// PxSSTS bits
#define AHCI_PXSSTS_DET   0x0F       // Device Detection
#define AHCI_PXSSTS_IPM   0xF00      // Interface Power Management

// Device types
#define AHCI_DEV_NULL      0
#define AHCI_DEV_SATA      1
#define AHCI_DEV_SATAPI    2
#define AHCI_DEV_SEMB      3
#define AHCI_DEV_PM        4

#define AHCI_SATA_SIG_ATA    0x00000101
#define AHCI_SATA_SIG_ATAPI  0xEB140101
#define AHCI_SATA_SIG_SEMB   0xC33C0101
#define AHCI_SATA_SIG_PM     0x96690101

// FIS Types
typedef enum {
    FIS_TYPE_REG_H2D   = 0x27, // Register FIS - host to device
    FIS_TYPE_REG_D2H   = 0x34, // Register FIS - device to host
    FIS_TYPE_DMA_ACT   = 0x39, // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
    FIS_TYPE_DATA      = 0x46, // Data FIS - bidirectional
    FIS_TYPE_BIST      = 0x58, // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS  = 0xA1, // Set device bits FIS - device to host
} FIS_TYPE;

typedef struct {
    u8  fis_type;
    u8  pmport:4;
    u8  rsv0:3;
    u8  c:1;
    u8  command;
    u8  featurel;
    u8  lba0;
    u8  lba1;
    u8  lba2;
    u8  device;
    u8  lba3;
    u8  lba4;
    u8  lba5;
    u8  featureh;
    u8  countl;
    u8  counth;
    u8  icc;
    u8  control;
    u8  rsv1[4];
} fis_reg_h2d_t;

typedef struct {
    u32 dba;
    u32 dbau;
    u32 rsv0;
    u32 dbc:22;
    u32 rsv1:9;
    u32 i:1;
} hba_prdt_entry_t;

typedef struct {
    u8  cfis[64];
    u8  acmd[16];
    u8  rsv[48];
    hba_prdt_entry_t prdt_entry[16];
} hba_cmd_table_t;

typedef struct {
    u8  cfl:5;
    u8  a:1;
    u8  w:1;
    u8  p:1;
    u8  r:1;
    u8  b:1;
    u8  c:1;
    u8  rsv0:1;
    u8  pmp:4;
    u16 prdtl;
    volatile u32 prdbc;
    u32 ctba;
    u32 ctbau;
    u32 rsv1[4];
} hba_cmd_header_t;

typedef struct {
    // 0x00
    u8  dsfis[28];
    u8  rsv0[4];
    // 0x20
    u8  psfis[24];
    u8  rsv1[8];
    // 0x40
    u8  rfis[24];
    u8  rsv2[12];
    // 0x58
    u8  sdbfis[16];
    // 0x64
    u8  ufis[64];
    // 0xA4
    u8  rsv3[0x60];
} hba_fis_t;

void ahci_init(void);
int ahci_identify(u8* port_base, u64 buffer_phys);
int ahci_read(u8* port_base, u64 startlba, u32 count, u64 buffer_phys);
int ahci_write(u8* port_base, u64 startlba, u32 count, u64 buffer_phys);

#endif
