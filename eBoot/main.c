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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <EmexToolchain/Boot/e64boot.h>
#include <fat.h>
#include <img4.h>
#include <img4_root.h>
#include <fw.h>
#include "disk.h"
#include "logo.h"
#include "poweroff.h"

#define EBOOT_VERSION   "eBoot 1.2.3"
#define LOGO_PATH       "/logo.img4"
#define LOGO_TYPE       "logo"

#define KCACHE_PATH     "/kernelcache.img4"
#define KCACHE_TYPE     "krnl"

#define UART_BASE       0x0020000000008000UL
#define UART_DATA       0x00
#define UART_STATUS     0x08
#define UART_TX_EMPTY   (1 << 1)

#define LINE_HEIGHT     16
#define MARGIN          2
#define GLYPH_WIDTH     8
#define GLYPH_BYTES     16

#define COLOR_BACK      0
#define GPU_PALETTE     0x1000

extern char __bss_start[] __attribute__((visibility("hidden")));
extern char __bss_end[] __attribute__((visibility("hidden")));
extern char __eboot_end[] __attribute__((visibility("hidden")));

extern unsigned char iso_font[256 * 16];

static struct {
    volatile uint8_t *fb;
    uint64_t width;
    uint64_t height;
    uint64_t stride;
    uint64_t line;
    uint64_t column;
    uint8_t color;
    uint64_t gpu;
} screen;

static int verbose;

static void uart_putc(char c)
{
    while(!(*(volatile uint64_t *)(UART_BASE + UART_STATUS) & UART_TX_EMPTY))
    {
    }
    *(volatile uint64_t *)(UART_BASE + UART_DATA) = (uint8_t)c;
}

static void screen_scroll(void)
{
    uint64_t shift = LINE_HEIGHT * screen.stride;
    uint64_t rest = (screen.height - LINE_HEIGHT) * screen.stride;

    for(uint64_t i = 0; i < rest; i++)
    {
        screen.fb[i] = screen.fb[i + shift];
    }
    for(uint64_t i = 0; i < shift; i++)
    {
        screen.fb[rest + i] = COLOR_BACK;
    }
}

static int is_logo(uint8_t pixel)
{
    return pixel != COLOR_BACK && pixel != screen.color;
}

static void screen_draw_glyph(unsigned char c,
                              uint64_t x,
                              uint64_t y)
{
    const uint8_t *glyph = &iso_font[c * GLYPH_BYTES];
    for(uint64_t row = 0; row < GLYPH_BYTES && y + row < screen.height; row++)
    {
        volatile uint8_t *line = screen.fb + (y + row) * screen.stride;
        for(uint64_t col = 0; col < GLYPH_WIDTH && x + col < screen.width; col++)
        {
            uint8_t under = line[x + col];
            if((glyph[row] >> col) & 1)
            {
                line[x + col] = is_logo(under) ? COLOR_BACK : screen.color;
            }
            else if(!is_logo(under))
            {
                line[x + col] = COLOR_BACK;
            }
        }
    }
}

static void screen_putc(char c)
{
    unsigned char ch = (unsigned char)c;
    if(screen.fb == 0 || ch == 0)
    {
        return;
    }

    if(ch == '\n')
    {
        screen.column = 0;
        if(screen.line + 1 >= screen.height / LINE_HEIGHT)
        {
            screen_scroll();
        }
        else
        {
            screen.line++;
        }
        return;
    }
    if(ch == '\r')
    {
        screen.column = 0;
        return;
    }

    uint64_t x = screen.column * GLYPH_WIDTH + MARGIN;
    uint64_t y = screen.line * LINE_HEIGHT + MARGIN;
    screen.column++;
    screen_draw_glyph(ch, x, y);
}

static void print(const char *s)
{
    while(*s)
    {
        if(screen.fb != 0)
        {
            screen_putc(*s);
        }
        else
        {
            if(*s == '\n')
            {
                uart_putc('\r');
            }
            uart_putc(*s);
        }
        s++;
    }
}

static void print_number(uint64_t value)
{
    char digits[20];
    int n = 0;
    do
    {
        digits[n++] = (char)('0' + value % 10);
        value /= 10;
    } while(value != 0);

    char text[21];
    for(int i = 0; i < n; i++)
    {
        text[i] = digits[n - 1 - i];
    }
    text[n] = 0;
    print(text);
}

static void palette_set(int entry, uint32_t rgb)
{
    if(screen.gpu == 0)
    {
        return;
    }
    if(verbose)
    {
        /*
         * for white mode:
         *
         * rgb ^= 0xFFFFFF;
         */
        rgb ^= 0x000000;
    }
    *(volatile uint64_t *)(screen.gpu + GPU_PALETTE + (uint64_t)entry * 8) = rgb;
}

static bool screen_init(struct e64_boot_info *info)
{
    if(info->fb_base == 0)
    {
        print("[eboot] error: no frame buffer base was provided by the BootROM\n");
        return false;
    }
    if(info->fb_format != 1)
    {
        print("[eboot] error: framebuffer format is not the one expected\n");
        return false;
    }
    if(info->fb_height < LINE_HEIGHT)
    {
        print("[eboot] error: framebuffer is to small\n");
        return false;
    }

    screen.fb = (volatile uint8_t *)info->fb_base;
    screen.width = info->fb_width;
    screen.height = info->fb_height;
    screen.stride = info->fb_stride;
    screen.gpu = info->gpu_regs;
    screen.column = 0;

    screen.line = info->screen_y > MARGIN ? (info->screen_y - MARGIN) / LINE_HEIGHT : 0;

    screen.color = 0xFF;
    uint64_t above = screen.line * LINE_HEIGHT + MARGIN;
    if(above > screen.height)
    {
        above = screen.height;
    }
    for(uint64_t i = above * screen.stride; i-- > 0;)
    {
        if(screen.fb[i] != COLOR_BACK)
        {
            screen.color = screen.fb[i];
            break;
        }
    }

    while(screen.line >= screen.height / LINE_HEIGHT)
    {
        screen_scroll();
        screen.line--;
    }

    for(int i = 0; i < 256; i++)
    {
        palette_set(i, (uint32_t)i * 0x010101);
    }
    if(!verbose)
    {
        for(uint64_t i = 0; i < screen.height * screen.stride; i++)
        {
            screen.fb[i] = COLOR_BACK;
        }
        screen.line = 0;
    }
    return true;
}

static int has_verbose(const struct e64_boot_info *info)
{
    const char *s = info->cmdline;
    size_t n = sizeof(info->cmdline);

    if(info->version < 5)
    {
        return 0;
    }
    for(size_t i = 0; i < n && s[i] != 0;)
    {
        while(i < n && s[i] == ' ')
        {
            i++;
        }
        size_t start = i;
        while(i < n && s[i] != 0 && s[i] != ' ')
        {
            i++;
        }
        if(i - start == 2 && s[start] == '-' && s[start + 1] == 'v')
        {
            return 1;
        }
    }
    return 0;
}

static uint32_t le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static const char *draw_logo(const uint8_t *data, size_t size)
{
    if(size < LOGO_HEADER_SIZE)
    {
        return "is too short";
    }
    for(int i = 0; i < 8; i++)
    {
        if(data[i] != (uint8_t)LOGO_MAGIC[i])
        {
            return "is no logo";
        }
    }
    uint64_t width = le32(data + 8);
    uint64_t height = le32(data + 12);
    uint64_t colors = le32(data + 16);
    if(width == 0 || height == 0 || width > LOGO_MAX_SIDE || height > LOGO_MAX_SIDE ||
       colors == 0 || colors > LOGO_MAX_COLORS || size != LOGO_HEADER_SIZE + colors * 3 + width * height)
    {
        return "is a logo with a wrong size";
    }

    const uint8_t *palette = data + LOGO_HEADER_SIZE;
    const uint8_t *pixels = palette + colors * 3;
    for(uint64_t i = 0; i < width * height; i++)
    {
        if(pixels[i] > colors)
        {
            return "uses a color it does not have";
        }
    }
    for(uint64_t k = 0; k < colors; k++)
    {
        palette_set((int)k + 1, ((uint32_t)palette[k * 3] << 16) | ((uint32_t)palette[k * 3 + 1] << 8) | palette[k * 3 + 2]);
    }

    uint64_t x0 = width < screen.width ? (screen.width - width) / 2 : 0;
    uint64_t y0 = height < screen.height ? (screen.height - height) / 2 : 0;
    uint64_t sx = width > screen.width ? (width - screen.width) / 2 : 0;
    uint64_t sy = height > screen.height ? (height - screen.height) / 2 : 0;
    for(uint64_t y = 0; y + sy < height && y0 + y < screen.height; y++)
    {
        const uint8_t *row = pixels + (y + sy) * width + sx;
        volatile uint8_t *line = screen.fb + (y0 + y) * screen.stride + x0;
        for(uint64_t x = 0; x + sx < width && x0 + x < screen.width; x++)
        {
            if(row[x] != 0)
            {
                line[x] = line[x] == screen.color ? COLOR_BACK : row[x];
            }
        }
    }
    return 0;
}

static struct disk disk = { 0 };
static struct fat_volume esp = { 0 };

static bool eboot_open_disk(const struct e64_boot_info *info)
{
    int err;
    err = disk_open(&disk, info->boot_device, info->boot_port);
    if(err != DISK_OK)
    {
        if(verbose)
        {
            print(err == DISK_ERR_KIND ? "[eboot] eBoot reads SATA disks only\n" : "[eboot] the boot disk is not there\n");
        }
        return false;
    }
    err = fat_mount_at(&esp, disk_read, &disk, info->partition_lba);
    if(err != FAT_OK)
    {
        print("[eboot] the ESP has no FAT file system\n");
        return false;
    }

    return true;
}

static uint64_t img4_load_cursor;
static bool eboot_load_img4(const struct e64_boot_info *info,
                            struct img4_image *image,
                            const char *path,
                            const char *type)
{
    int err;
    uint64_t size;
    if(img4_load_cursor == 0)
    {
        img4_load_cursor = ((uint64_t)__eboot_end + 0x1FFF) & ~0x1FFFUL;
    }
    uint64_t at = img4_load_cursor;
    uint64_t end = info->fb_base > at ? info->fb_base : info->memory_size;
    if(end <= at)
    {
        print("[eboot] there is no memory to load img4 at '");
        print(path);
        print("\n");
        return false;
    }
    err = fat_load(&esp, path, (void *)at, end - at, &size);
    if(err != FAT_OK)
    {
        if(err != FAT_ERR_NOT_FOUND || verbose)
        {
            print(err == FAT_ERR_NOT_FOUND ? "[eboot] no '" : "[eboot] '");
            print(path);
            print(err == FAT_ERR_NOT_FOUND ? "' on the ESP\n" : "' could not be read\n");
            print("\n");
        }
        return false;
    }

    struct img4_root root;
    root.public_key = img4_root_public_key;
    root.subject = img4_root_subject;
    root.subject_size = sizeof(img4_root_subject);
    err = img4_verify((const uint8_t *)at, size, type, &root, &img4_this_device, image);
    if(err != IMG4_OK)
    {
        print("[eboot] '");
        print(path);
        print("' ");
        print(img4_error(err));
        print(", not showing it\n");
        return false;
    }

    img4_load_cursor = (at + size + 0x1FFF) & ~0x1FFFUL;
    if(verbose)
    {
        print("[eboot] loaded '");
        print(path);
        print("' (codesignature valid)\n");
    }

    return true;
}

void eboot_main(struct e64_boot_info *info)
{
    for(volatile char *p = __bss_start; p < __bss_end; p++)
    {
        *p = 0;
    }

    if(info == 0 || info->magic != E64BOOT_INFO_MAGIC)
    {
        print("[eboot] BootROM gave no boot info\n");
        power_off();
    }

    verbose = has_verbose(info);
    bool success = screen_init(info);
    if(verbose)
    {
        print(EBOOT_VERSION "\n");
    }

    /* first open the boot disk*/
    if(!eboot_open_disk(info))
    {
        goto fail;
    }

    /* find ecid */
    img4_this_device.ecid = mmio_read64(EMEX64_CPUID_REG_ECID);

    /* now we load the images first, they must all be valid */
    struct img4_image logo_image;
    struct img4_image kernelcache_image;

    if(verbose)
    {
        print("[eboot] loading img4 payloads\n");
    }
    if(!eboot_load_img4(info, &logo_image, LOGO_PATH, LOGO_TYPE) ||
       !eboot_load_img4(info, &kernelcache_image, KCACHE_PATH, KCACHE_TYPE))
    {
        goto fail;
    }

    /* acting uppon loaded images */
    if(verbose)
    {
        print("[eboot] setting up img4 payloads\n");
    }
    if(success)
    {
        if(verbose)
        {
            print("[eboot] showing logo\n");
        }
        const char *why = draw_logo(logo_image.data, logo_image.size);
        if(why != 0)
        {
            print("[eboot] failed to show logo: '");
            print(why);
            print("'\n");
            return;
        }
    }

    if(verbose)
    {
        print("[eboot] jumping to kernelcache entry\n");
    }
    void (*kernelcache_entry)(void) = (void*)kernelcache_image.data;
    kernelcache_entry();

    print("[eboot] error: returned from kernelcache entry\n");

fail:
    for(;;)
    {
        __asm__ __volatile__("hlt");
    }
}

__attribute__((naked, section(".text.entry")))
void _start(void)
{
    __asm__ __volatile__(
        "add r1, pc, __stack_top@pcrel\n\t"
        "mov sp, r1\n\t"
        "bl eboot_main@plt");
}
