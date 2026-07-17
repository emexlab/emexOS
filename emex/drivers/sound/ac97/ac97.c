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

#include "ac97.h"

#include <kernel/pci/device.h>
#include <kernel/pci/config.h>
#include <kernel/include/ports.h>
#include <kernel/include/reqs.h>
#include <kernel/mem/phys/physmem.h>
#include <kernel/mem/lib/main.h>
#include <kernel/graph/lib/string.h>
#include <kernel/communication/serial.h>
#include <kernel/arch/x86_64/exceptions/irq.h>

#if ENABLE_ULIME
#include <kernel/proc/scheduler.h>
extern scheduler_t *scheduler;
#endif

static struct {
    u16 nam_base;
    u16 nabm_base;

    ac97_bdl_entry_t *bdl;
    u64               bdl_phys;

    u8  *bufs[AC97_BDL_SIZE];
    u64  bufs_phys[AC97_BDL_SIZE];

    u8  write_idx;
    u8  irq_line;
    int present;
} dev;

static inline u16 nam_read(u8 reg)            { return inw(dev.nam_base  + reg); }
static inline void nam_write(u8 reg, u16 val) { outw(dev.nam_base  + reg, val);  }

static inline u8   nabm_read8(u8 reg)             { return inb(dev.nabm_base + reg); }
static inline void nabm_write8(u8 reg, u8 val)    { outb(dev.nabm_base + reg, val);  }
static inline void nabm_write32(u8 reg, u32 val)  { outl(dev.nabm_base + reg, val);  }

static void ac97_irq_handler(cpu_state_t *state)
{
    (void)state;

    u16 sr = (u16)inw(dev.nabm_base + AC97_NABM_PCM_OUT_SR);

    if (!(sr & (AC97_SR_BCIS | AC97_SR_LVBCI)))
        return;

    outw(dev.nabm_base + AC97_NABM_PCM_OUT_SR, sr & (AC97_SR_BCIS | AC97_SR_LVBCI));

	#if ENABLE_ULIME
	    if (scheduler != NULL)
	        scheduler->ticks =	 scheduler->quantum;
	#endif
}

static int codec_cold_reset(void)
{
    if (inl(dev.nabm_base + AC97_NABM_GLOB_STS) & AC97_GLOB_STS_ANY_CODEC)
        return 0;

    u32 glob = inl(dev.nabm_base + AC97_NABM_GLOB_CNT);

    glob &= ~AC97_GLOB_CNT_COLD;
    outl(dev.nabm_base + AC97_NABM_GLOB_CNT, glob);
    for (volatile int i = 0; i < 100000; i++) __asm__ volatile("nop");

    glob |= AC97_GLOB_CNT_COLD;
    outl(dev.nabm_base + AC97_NABM_GLOB_CNT, glob);

    /* warm reset after cold — some codecs require it */
    glob |= AC97_GLOB_CNT_WARM;
    outl(dev.nabm_base + AC97_NABM_GLOB_CNT, glob);
    for (volatile int i = 0; i < 100000; i++) __asm__ volatile("nop");
    glob &= ~AC97_GLOB_CNT_WARM;
    outl(dev.nabm_base + AC97_NABM_GLOB_CNT, glob);

    for (int i = 0; i < 1000000; i++) {
        if (inl(dev.nabm_base + AC97_NABM_GLOB_STS) & AC97_GLOB_STS_ANY_CODEC)
            return 0;
        __asm__ volatile("nop");
    }

    return -1;
}

static int bdl_alloc(void)
{
    u64 hhdm = hhdm_request.response->offset;

    /* 32 × 8 bytes = 256 bytes; one page is sufficient */
    dev.bdl_phys = physmem_alloc_to(1);
    if (!dev.bdl_phys)
        return -1;

    dev.bdl = (ac97_bdl_entry_t *)(dev.bdl_phys + hhdm);
    memset(dev.bdl, 0, sizeof(ac97_bdl_entry_t) * AC97_BDL_SIZE);

    for (int i = 0; i < AC97_BDL_SIZE; i++) {
        dev.bufs_phys[i] = physmem_alloc_to(AC97_BDL_BUF_PAGES);
        if (!dev.bufs_phys[i])
            return -1;

        dev.bufs[i] = (u8 *)(dev.bufs_phys[i] + hhdm);
        memset(dev.bufs[i], 0, AC97_BDL_BUF_BYTES);

        /* DMA address must fit in 32 bits — AC97 has no 64-bit DMA */
        dev.bdl[i].phys_addr = (u32)dev.bufs_phys[i];
        dev.bdl[i].samples   = (AC97_BDL_BUF_SAMPLES & 0xFFFF) | AC97_BDL_FLAG_IOC;
    }

    return 0;
}

int ac97_init(void)
{
    memset(&dev, 0, sizeof(dev));

    if (!hhdm_request.response) {
        log("[AC97]", "no HHDM response\n", error);
        return -1;
    }

    pci_device_t *pci = NULL;

    static const u16 ich_ids[] = {
        AC97_DEV_ICH,  AC97_DEV_ICH0, AC97_DEV_ICH2,
        AC97_DEV_ICH3, AC97_DEV_ICH4, AC97_DEV_ICH5,
        AC97_DEV_ICH6, AC97_DEV_ICH7, 0
    };

    for (int i = 0; ich_ids[i] && !pci; i++)
        pci = pci_device_find_by_vendor(AC97_VENDOR_INTEL, ich_ids[i]);

    if (!pci)
        pci = pci_device_find_by_vendor(AC97_VENDOR_ENSONIQ, AC97_DEV_VMWARE);

    if (!pci)
        pci = pci_device_find_by_class(AC97_PCI_CLASS, AC97_PCI_SUBCLASS);

    if (!pci) {
        log("[AC97]", "no controller found\n", warning);
        return -1;
    }

    /* I/O space + bus mastering */
    u16 cmd = pci_config_read_word(pci->bus, pci->device, pci->function, 0x04);
    cmd |= (1u << 0) | (1u << 2);
    pci_config_write_word(pci->bus, pci->device, pci->function, 0x04, cmd);

    /* BAR0 = NAM, BAR1 = NABM; both I/O — strip the I/O flag bits */
    u32 bar0 = pci_config_read(pci->bus, pci->device, pci->function, 0x10);
    u32 bar1 = pci_config_read(pci->bus, pci->device, pci->function, 0x14);
    dev.nam_base  = (u16)(bar0 & ~0x3u);
    dev.nabm_base = (u16)(bar1 & ~0x3u);

    if (!dev.nam_base || !dev.nabm_base) {
        log("[AC97]", "invalid BARs\n", error);
        return -1;
    }

    /* IRQ line from PCI config — register before enabling interrupts */
    dev.irq_line = pci_config_read_byte(pci->bus, pci->device, pci->function, 0x3C);
    if (dev.irq_line && dev.irq_line < 16) {
        irq_register_handler(dev.irq_line, ac97_irq_handler);
        log("[AC97]", "IRQ registered\n", d);
    }

    if (codec_cold_reset() != 0) {
        log("[AC97]", "codec did not become ready after reset\n", error);
        return -1;
    }

    nam_write(AC97_NAM_RESET, 0);
    for (volatile int i = 0; i < 100000; i++) __asm__ volatile("nop");

    nam_write(AC97_NAM_MASTER_VOL,    AC97_VOL_MAX);
    nam_write(AC97_NAM_HEADPHONE_VOL, AC97_VOL_MAX);
    nam_write(AC97_NAM_PCM_VOL,       AC97_VOL_MAX);

    u16 ext_id = nam_read(AC97_NAM_EXT_AUDIO_ID);
    if (ext_id & 0x0001) {
        u16 sts = nam_read(AC97_NAM_EXT_AUDIO_STS);
        nam_write(AC97_NAM_EXT_AUDIO_STS, sts | 0x0001);
    }

    nam_write(AC97_NAM_PCM_RATE, AC97_DEFAULT_RATE);

    if (bdl_alloc() != 0) {
        log("[AC97]", "BDL allocation failed\n", error);
        return -1;
    }

    nabm_write8(AC97_NABM_PCM_OUT_CR, 0);
    nabm_write8(AC97_NABM_PCM_OUT_CR, AC97_CR_RR);
    for (volatile int i = 0; i < 10000; i++) __asm__ volatile("nop");

    nabm_write32(AC97_NABM_PCM_OUT_BDBAR, (u32)dev.bdl_phys);
    nabm_write8(AC97_NABM_PCM_OUT_LVI, 0);

    /* enable buffer-completion and last-valid-buffer interrupts */
    nabm_write8(AC97_NABM_PCM_OUT_CR, AC97_CR_IOCE | AC97_CR_LVBIE);

    dev.write_idx = 0;
    dev.present   = 1;

    {
        char buf[64];
        char hexbuf[32];
        buf[0] = '\0';
        str_append(buf, "NAM=0x");
        str_from_hex(hexbuf, dev.nam_base);
        str_append(buf, hexbuf);
        str_append(buf, " NABM=0x");
        str_from_hex(hexbuf, dev.nabm_base);
        str_append(buf, hexbuf);
        str_append(buf, "\n");
        log("[AC97]", buf, success);
    }

    return 0;
}

int ac97_write(const void *buf, size_t len)
{
    if (!dev.present || !buf || !len)
        return -1;

    const u8 *src     = (const u8 *)buf;
    size_t    written = 0;

    while (written < len) {
        u8  civ        = nabm_read8(AC97_NABM_PCM_OUT_CIV);
        u8  ring_slots = (u8)((dev.write_idx - civ + AC97_BDL_SIZE) % AC97_BDL_SIZE);

        if (ring_slots == AC97_BDL_SIZE - 1)
            break;

        size_t chunk = len - written;
        if (chunk > AC97_BDL_BUF_BYTES)
            chunk = AC97_BDL_BUF_BYTES;

        memcpy(dev.bufs[dev.write_idx], src + written, chunk);

        if (chunk < AC97_BDL_BUF_BYTES)
            memset(dev.bufs[dev.write_idx] + chunk, 0, AC97_BDL_BUF_BYTES - chunk);

        dev.bdl[dev.write_idx].samples =
            ((u32)(chunk / 2) & 0xFFFF) | AC97_BDL_FLAG_IOC;

        nabm_write8(AC97_NABM_PCM_OUT_LVI, dev.write_idx);

        u16 sr = (u16)inw(dev.nabm_base + AC97_NABM_PCM_OUT_SR);
        if (sr & AC97_SR_DCH)
            nabm_write8(AC97_NABM_PCM_OUT_CR,
                        AC97_CR_RPBM | AC97_CR_IOCE | AC97_CR_LVBIE);

        written       += chunk;
        dev.write_idx  = (dev.write_idx + 1) % AC97_BDL_SIZE;
    }

    return (int)written;
}

void ac97_set_volume(u8 master_atten, u8 pcm_atten)
{
    if (!dev.present)
        return;

    u8  m    = master_atten & 0x3F;
    u8  p    = pcm_atten    & 0x3F;
    u16 mvol = ((u16)m << 8) | m;
    u16 pvol = ((u16)p << 8) | p;

    nam_write(AC97_NAM_MASTER_VOL, mvol);
    nam_write(AC97_NAM_PCM_VOL,    pvol);
}

int ac97_set_rate(u32 hz)
{
    if (!dev.present)
        return -1;

    u16 ext_id = nam_read(AC97_NAM_EXT_AUDIO_ID);
    if (!(ext_id & 0x0001) && hz != 48000) {
        log("[AC97]", "codec lacks VRA, only 48000 Hz supported\n", warning);
        return -1;
    }

    nam_write(AC97_NAM_PCM_RATE, (u16)hz);

    u16 actual = nam_read(AC97_NAM_PCM_RATE);
    if (actual != (u16)hz) {
        log("[AC97]", "rate not accepted by codec\n", warning);
        return -1;
    }

    return 0;
}

int ac97_present(void)
{
    return dev.present;
}

static const i16 sine_table[256] = {
       0,   804,  1608,  2410,  3212,  4011,  4808,  5602,
    6393,  7179,  7962,  8739,  9512, 10278, 11039, 11793,
   12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530,
   18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
   23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
   27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
   30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971,
   32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
   32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285,
   32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
   30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683,
   27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
   23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868,
   18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279,
   12539, 11793, 11039, 10278,  9512,  8739,  7962,  7179,
    6393,  5602,  4808,  4011,  3212,  2410,  1608,   804,
       0,  -804, -1608, -2410, -3212, -4011, -4808, -5602,
   -6393, -7179, -7962, -8739, -9512,-10278,-11039,-11793,
  -12539,-13279,-14010,-14732,-15446,-16151,-16846,-17530,
  -18204,-18868,-19519,-20159,-20787,-21403,-22005,-22594,
  -23170,-23731,-24279,-24811,-25329,-25832,-26319,-26790,
  -27245,-27683,-28105,-28510,-28898,-29268,-29621,-29956,
  -30273,-30571,-30852,-31113,-31356,-31580,-31785,-31971,
  -32137,-32285,-32412,-32521,-32609,-32678,-32728,-32757,
  -32767,-32757,-32728,-32678,-32609,-32521,-32412,-32285,
  -32137,-31971,-31785,-31580,-31356,-31113,-30852,-30571,
  -30273,-29956,-29621,-29268,-28898,-28510,-28105,-27683,
  -27245,-26790,-26319,-25832,-25329,-24811,-24279,-23731,
  -23170,-22594,-22005,-21403,-20787,-20159,-19519,-18868,
  -18204,-17530,-16846,-16151,-15446,-14732,-14010,-13279,
  -12539,-11793,-11039,-10278, -9512, -8739, -7962, -7179,
   -6393, -5602, -4808, -4011, -3212, -2410, -1608,  -804,
};

int ac97_beep(u32 freq_hz, u32 duration_ms)
{
    if (!dev.present)
        return -1;

    if (!freq_hz || !duration_ms)
        return 0;

    const u32 rate         = AC97_DEFAULT_RATE;          /* 44100 Hz        */
    const i16 amplitude    = 16383;                      /* half full-scale */

    u32 total_frames = (rate / 1000) * duration_ms;

    u64 step_fp = ((u64)freq_hz * 256u * 65536u) / rate;

    u32 phase_acc = 0;   /* 16.16 fixed-point phase accumulator */

    i16 buf[AC97_BDL_BUF_BYTES / sizeof(i16)];

    const u32 frames_per_buf = AC97_BDL_BUF_BYTES / 4;

    int total_written = 0;
    u32 frames_done   = 0;

    while (frames_done < total_frames) {
        u32 this_frames = total_frames - frames_done;
        if (this_frames > frames_per_buf)
            this_frames = frames_per_buf;

        for (u32 i = 0; i < this_frames; i++) {
            u8  idx     = (u8)((phase_acc >> 16) & 0xFF);
            i16 sample  = (i16)(((i32)sine_table[idx] * amplitude) >> 15);

            buf[i * 2 + 0] = sample;
            buf[i * 2 + 1] = sample;

            phase_acc += (u32)step_fp;
        }

        if (this_frames < frames_per_buf) {
            u32 pad = (frames_per_buf - this_frames) * 2;
            for (u32 p = this_frames * 2; p < this_frames * 2 + pad; p++)
                buf[p] = 0;
        }

        int n = ac97_write(buf, this_frames * 4);
        if (n > 0)
            total_written += n;

        frames_done += this_frames;
    }

    return total_written;
}
