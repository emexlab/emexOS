/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
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

#include <fw.h>
#include <pcie.h>
#include <blk.h>
#include <sata.h>
#include <usb.h>
#include <fat.h>
#include <img4.h>
#include <img4_root.h>
#include <nvram.h>
#include <EmexToolchain/Boot/e64boot.h>

#define SECTOR              E64BOOT_SECTOR_SIZE
#define MAX_ENTRY_BYTES     (64 * 1024)

#define GUI_LINE_HEIGHT     16
#define GUI_MARGIN          2

#define ESP_BOOTLOADER      "/bootloader.img4"
#define BOOTLOADER_TYPE     "ebot"

#define NVRAM_BASE          (EMEX64_MMIO_BASE + EMEX64_PAGE_SIZE * 11)
#define NVRAM_BYTES         0x10000UL

static u8 sector_buf[SECTOR];
static u8 nvram_buf[NVRAM_BYTES];
static u8 entry_buf[MAX_ENTRY_BYTES];

static struct e64_boot_info boot_info;
extern unsigned short gui_line, gui_column;

static int read_gpt_header(struct blkdev *d,
                           u64 lba,
                           struct gpt_header *h)
{
    int err = d->read(d, lba, 1, sector_buf);
    if(err)
    {
        fw_printf("[BootROM] %s: reading LBA %lu failed (%s)\n", d->name, lba, d->strerror(d, err));
        return -1;
    }

    fw_memcpy(h, sector_buf, sizeof(*h));
    if((fw_memcmp(h->signature, GPT_SIGNATURE, 8) != 0) &&
       (h->header_size < GPT_HEADER_SIZE || h->header_size > SECTOR))
    {
        return -1;
    }

    u32 want = h->header_crc32;
    ((struct gpt_header *)sector_buf)->header_crc32 = 0;
    if(fw_crc32_update(0, sector_buf, h->header_size) != want)
    {
        fw_printf("[BootROM] %s: GPT header at LBA %lu has a bad checksum\n", d->name, lba);
        return -1;
    }

    if(h->entry_size < sizeof(struct gpt_entry) ||
       h->entry_count == 0 ||
       (u64)h->entry_count * h->entry_size > MAX_ENTRY_BYTES)
    {
        fw_printf("[BootROM] %s: unsupported GPT entry layout\n", d->name);
        return -1;
    }

    return 0;
}

static int read_gpt(struct blkdev *d,
                    struct gpt_header *h)
{
    if(read_gpt_header(d, 1, h) != 0)
    {
        if(d->sectors < 2 || read_gpt_header(d, d->sectors - 1, h) != 0)
        {
            return -1;
        }
        fw_printf("[BootROM] %s: using the backup GPT\n", d->name);
    }

    u64 bytes = (u64)h->entry_count * h->entry_size;
    u64 sectors = (bytes + SECTOR - 1) / SECTOR;
    int err = d->read(d, h->entries_lba, sectors, entry_buf);
    if(err)
    {
        fw_printf("[BootROM] %s: reading the partition table failed (%s)\n", d->name, d->strerror(d, err));
        return -1;
    }

    if(fw_crc32_update(0, entry_buf, bytes) != h->entries_crc32)
    {
        fw_printf("[BootROM] %s: partition table has a bad checksum\n", d->name);
        return -1;
    }

    return 0;
}

static int check_header(struct blkdev *d,
                        struct e64_boot_header *bh,
                        struct gpt_entry *e)
{
    u64 part_bytes = (e->last_lba - e->first_lba) * SECTOR;
    u64 mem = fw_ram_end();
    u64 fw_end = ((u64)__bss_end + EMEX64_PAGE_SIZE - 1) & ~(EMEX64_PAGE_SIZE - 1);
    u32 want = bh->header_crc32;

    bh->header_crc32 = 0;
    if(fw_memcmp(bh->magic, E64BOOT_MAGIC, 8) != 0)
    {
        fw_printf("[BootROM] %s: boot partition has no emex64 boot header\n", d->name);
        return -1;
    }
    if(fw_crc32_update(0, bh, sizeof(*bh)) != want)
    {
        fw_printf("[BootROM] %s: boot header checksum mismatch\n", d->name);
        return -1;
    }
    bh->header_crc32 = want;

    if(bh->version != E64BOOT_VERSION || bh->header_size != SECTOR)
    {
        fw_printf("[BootROM] %s: unsupported boot header version %u\n", d->name, bh->version);
        return -1;
    }
    if(bh->image_size == 0 || bh->image_size > part_bytes || bh->memory_size < bh->image_size)
    {
        fw_printf("[BootROM] %s: boot image size is invalid\n", d->name);
        return -1;
    }
    if(bh->load_address & (EMEX64_PAGE_SIZE - 1))
    {
        fw_printf("[BootROM] %s: load address 0x%lx is not page aligned\n", d->name, bh->load_address);
        return -1;
    }

    u64 footprint = (bh->image_size + SECTOR - 1) & ~(u64)(SECTOR - 1);
    if(footprint < bh->memory_size)
    {
        footprint = bh->memory_size;
    }
    if(bh->load_address < fw_end || bh->load_address > mem || footprint > mem - bh->load_address)
    {
        fw_printf("[BootROM] %s: image 0x%lx-0x%lx does not fit (firmware ends at 0x%lx, memory 0x%lx)\n",
                  d->name, bh->load_address, bh->load_address + bh->memory_size, fw_end, mem);
        return -1;
    }
    if(bh->entry < bh->load_address || bh->entry - bh->load_address >= bh->image_size)
    {
        fw_printf("[BootROM] %s: entry point 0x%lx is outside the image\n", d->name, bh->entry);
        return -1;
    }
    if(bh->text_size != 0 &&
       ((bh->text_size & (EMEX64_PAGE_SIZE - 1)) || bh->text_size > bh->memory_size ||
        bh->entry - bh->load_address >= bh->text_size))
    {
        fw_printf("[BootROM] %s: text size 0x%lx is invalid\n", d->name, bh->text_size);
        return -1;
    }

    return 0;
}

static void read_boot_args(char *out,
                           u64 size)
{
    const char *value;
    size_t value_size, offset, length;
    u64 bytes = mmio_read64(NVRAM_BASE + NVRAM_BYTES);

    out[0] = 0;
    if(bytes == 0 || bytes > NVRAM_BYTES)
    {
        bytes = NVRAM_BYTES;
    }
    for(u64 i = 0; i + 8 <= bytes; i += 8)
    {
        *(u64 *)(nvram_buf + i) = mmio_read64(NVRAM_BASE + i);
    }

    if(nvram_common(nvram_buf, bytes, &offset, &length) != 0 ||
       nvram_get(nvram_buf + offset, length, "boot-args", &value, &value_size) != 0)
    {
        return;
    }
    if(value_size > size - 1)
    {
        value_size = size - 1;
    }
    fw_memcpy(out, value, value_size);
    out[value_size] = 0;
    fw_printf("[BootROM] boot-args %s\n", out);
}

static const u8 esp_part_type_guid[16] = {
    0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11,
    0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B,
};

static int esp_read(void *context,
                    uint64_t lba,
                    uint32_t count,
                    void *buffer)
{
    struct blkdev *d = context;
    return d->read(d, lba, count, buffer);
}

static void try_disk(struct blkdev *d)
{
    struct gpt_header h;
    struct e64_boot_header bh;

    if(read_gpt(d, &h) != 0)
    {
        fw_printf("[BootROM] %s: no valid GPT\n", d->name);
        return;
    }

    for(u32 i = 0; i < h.entry_count; i++)
    {
        struct gpt_entry *e = (struct gpt_entry *)(entry_buf + (u64)i * h.entry_size);
        if(fw_memcmp(e->type_guid, esp_part_type_guid, 16) == 0)
        {
            struct fat_volume esp;
            u64 load = ((u64)__bss_end + EMEX64_PAGE_SIZE - 1) & ~(EMEX64_PAGE_SIZE - 1);
            u64 size;
            int err;

            if(e->last_lba < e->first_lba || e->last_lba >= d->sectors)
            {
                return;
            }

            err = fat_mount_at(&esp, esp_read, d, e->first_lba);
            if(err != FAT_OK)
            {
                fw_printf("[BootROM] %s: ESP has no FAT file system (error %u)\n", d->name, (u32)-err);
                return;
            }

            err = fat_load(&esp, ESP_BOOTLOADER, (void *)load, fw_ram_end() - load, &size);
            if(err != FAT_OK)
            {
                fw_printf("[BootROM] %s: %s not loaded (error %u)\n", d->name, ESP_BOOTLOADER, (u32)-err);
                return;
            }

            fw_printf("[BootROM] loaded %s: ", ESP_BOOTLOADER);
            fw_put_size(size);
            fw_printf(" to 0x%lx\n", load);

            /* lets hope that it is valid */
            struct img4_image image;
            err = img4_verify((const u8 *)load, size, BOOTLOADER_TYPE, &img4_rom_root, &img4_this_device, &image);
            if(err != IMG4_OK)
            {
                fw_printf("[BootROM] %s: %s %s, not booting it\n", d->name, ESP_BOOTLOADER, img4_error(err));
                return;
            }

            char name[sizeof(boot_info.name)];
            u64 name_size = image.description_size < sizeof(name) - 1 ? image.description_size : sizeof(name) - 1;
            fw_memcpy(name, image.description, name_size);
            name[name_size] = 0;
            memmove((void *)load, image.data, image.size);
            size = image.size;
            fw_printf("[BootROM] '%s' is validly signed by vendor\n", name);

            fw_memset(&boot_info, 0, sizeof(boot_info));
            boot_info.magic = E64BOOT_INFO_MAGIC;
            boot_info.version = E64BOOT_INFO_VERSION;
            boot_info.memory_size = mmio_read64(EMEX64_MC_REG_SIZE);
            boot_info.cores = (mmio_read64(EMEX64_APIC_REG_VERSION) >> 8) & 0xFF;
            boot_info.firmware_end = (u64)__bss_end;
            boot_info.image_start = load;
            boot_info.image_end = load + size;
            boot_info.boot_device = ((u64)d->kind << 32) | ((u64)d->ctrl->bus << 16) | ((u64)d->ctrl->dev << 8) | d->ctrl->fn;
            boot_info.boot_port = (u64)d->port;
            boot_info.partition_lba = e->first_lba;
            boot_info.partition_sectors = e->last_lba - e->first_lba + 1;
            fw_memcpy(boot_info.name, name, sizeof(name));
            read_boot_args(boot_info.cmdline, sizeof(boot_info.cmdline));

            fw_printf("[BootROM] jumping to 0x%lx\n", load);

            if(gui_fb)
            {
                boot_info.screen_y = (u64)(gui_line + (gui_column != 0)) * GUI_LINE_HEIGHT + GUI_MARGIN;
                boot_info.fb_base = gui_fb;
                boot_info.fb_width = gui_width;
                boot_info.fb_height = gui_height;
                boot_info.fb_stride = gui_width;
                boot_info.fb_format = 1;
                boot_info.gpu_regs = display_bar;
            }

            fw_jump(load, &boot_info);

            continue;
        }
    }

    fw_printf("[BootROM] %s: nothing bootable\n", d->name);
}

void fw_boot_main(void)
{
    /* scanning storage devices */
    if(pcie_init() <= 0)
    {
        return;
    }
    sata_init();
    usb_init();

    /* hopefully there are block devices to boot from */
    if(blk_count() == 0)
    {
        fw_printf("[BootROM] no disks\n");
        return;
    }

    /* trying to boot from each block device */
    for(int i = 0; i < blk_count(); i++)
    {
        try_disk(blk_get(i));
    }

    /* ouweeeeee */
    fw_printf("[BootROM] no bootable disk found\n");
}
