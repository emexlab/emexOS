/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: disk.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "disk.h"
#include "mbr.h"
#include "partition.h"

#include <kernel/graph/graphics.h>
#include <drivers/storage/ata/disk.h>
#include <drivers/storage/ahci/ahci.h>

void disks_init()
{
	ata_init();
	/*
	 * TODO:
	 * disk layer here
	 */

	//ahci_init();

    int part_result = partition_init();

    if (part_result != 0 || partition_needs_format())
    {
        log("[DISK]", "no valid partition found\n", warning);
        log("[DISK]", "run 'install' to set up the disk\n", warning);
    }

}