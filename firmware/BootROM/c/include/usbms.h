/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.1
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.1
 * (EPSL 1.1).
 *
 * Private use and modification are permitted under the license.
 * Redistribution of source or binaries, modified or unmodified,
 * requires prior written permission from emexLabs.
 *
 * External deployment is subject to the source-availability
 * requirements and other conditions of the license.
 *
 * Provided "AS IS", without warranty, to the extent permitted by law.
 *
 * See LICENSE for the full license terms.
 */

#ifndef FW_USBMS_H
#define FW_USBMS_H

#include <fw.h>
#include <usb.h>
#include <blk.h>

#define USBMS_MAX_DISKS     8

/* errors returned through the block device, on top of the USB_ERR_* codes */
#define USBMS_ERR_FAILED    0x10    /* CSW: command failed (see sense) */
#define USBMS_ERR_PHASE     0x11    /* CSW: phase error */
#define USBMS_ERR_CSW       0x12    /* missing or malformed CSW */

struct usbms_disk {
    struct blkdev blk;          /* first member */
    struct usb_device *dev;
    u32 tag;
    u32 block_size;
    u64 blocks;
    u8 sense_key, asc, ascq;    /* of the last failed command */
    char vendor[9];
    char product[17];
};

/* binds a bulk-only SCSI interface, returns 0 when a disk was registered */
int usbms_probe(struct usb_device *dev);

#endif /* FW_USBMS_H */
