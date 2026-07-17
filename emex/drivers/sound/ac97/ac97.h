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

#ifndef AC97_H
#define AC97_H

#include <types.h>

#define AC97_VENDOR_INTEL   0x8086
#define AC97_DEV_ICH        0x2415
#define AC97_DEV_ICH0       0x2425
#define AC97_DEV_ICH2       0x2445
#define AC97_DEV_ICH3       0x2485
#define AC97_DEV_ICH4       0x24C5
#define AC97_DEV_ICH5       0x24D5
#define AC97_DEV_ICH6       0x266E
#define AC97_DEV_ICH7       0x27DE
#define AC97_DEV_VMWARE     0x7187
#define AC97_VENDOR_ENSONIQ 0x1274

#define AC97_PCI_CLASS    0x04
#define AC97_PCI_SUBCLASS 0x01


#define AC97_NAM_RESET          0x00
#define AC97_NAM_MASTER_VOL     0x02
#define AC97_NAM_HEADPHONE_VOL  0x04
#define AC97_NAM_PCM_VOL        0x18
#define AC97_NAM_PCM_RATE       0x2C
#define AC97_NAM_EXT_AUDIO_ID   0x28
#define AC97_NAM_EXT_AUDIO_STS  0x2A

#define AC97_NABM_PCM_OUT_BDBAR 0x10
#define AC97_NABM_PCM_OUT_CIV   0x14
#define AC97_NABM_PCM_OUT_LVI   0x15
#define AC97_NABM_PCM_OUT_SR    0x16
#define AC97_NABM_PCM_OUT_CR    0x1B
#define AC97_NABM_GLOB_CNT      0x2C
#define AC97_NABM_GLOB_STS      0x30

/* PCM Out Control Register */
#define AC97_CR_RPBM  (1 << 0)  /* run/pause DMA */
#define AC97_CR_RR    (1 << 1)  /* reset channel registers */
#define AC97_CR_LVBIE (1 << 2)  /* last-valid-buffer interrupt enable */
#define AC97_CR_FEIE  (1 << 3)  /* FIFO error interrupt enable */
#define AC97_CR_IOCE  (1 << 4)  /* buffer-completion interrupt enable */

#define AC97_SR_DCH   (1 << 0)
#define AC97_SR_CELV  (1 << 1)
#define AC97_SR_LVBCI (1 << 2)
#define AC97_SR_BCIS  (1 << 3)
#define AC97_SR_FIFOE (1 << 4)

#define AC97_GLOB_CNT_GIE  (1 << 0)
#define AC97_GLOB_CNT_COLD (1 << 1)
#define AC97_GLOB_CNT_WARM (1 << 2)

#define AC97_GLOB_STS_PRES1     (1 << 7)
#define AC97_GLOB_STS_PRES2     (1 << 8)
#define AC97_GLOB_STS_ANY_CODEC (AC97_GLOB_STS_PRES1 | AC97_GLOB_STS_PRES2)

#define AC97_VOL_MUTE  (1 << 15)
#define AC97_VOL_MAX   0x0000

#define AC97_BDL_SIZE        32
#define AC97_BDL_BUF_PAGES   1
#define AC97_BDL_BUF_BYTES   4096
#define AC97_BDL_BUF_SAMPLES (AC97_BDL_BUF_BYTES / 2)

#define AC97_BDL_FLAG_IOC  (1u << 31)
#define AC97_BDL_FLAG_BUP  (1u << 30)

#define AC97_DEFAULT_RATE  44100

typedef struct {
    u32 phys_addr;
    u32 samples;
} ac97_bdl_entry_t;

int  ac97_init(void);
int  ac97_write(const void *buf, size_t len);
void ac97_set_volume(u8 master_atten, u8 pcm_atten);
int  ac97_set_rate(u32 hz);
int  ac97_present(void);

int  ac97_beep(u32 freq_hz, u32 duration_ms);

void ac97_test(void);

#endif
