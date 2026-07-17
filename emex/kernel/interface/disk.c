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
