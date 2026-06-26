/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: wav.c
 * CREATED BY: Offihito
 * MODIFIED BY: emex
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define AUDIO_DEV    "/dev/audio0"
#define IO_BUF_SIZE  65536

static uint8_t  hdr_buf[256];
static int16_t  pcm_buf[IO_BUF_SIZE / 2];

static int xread(int fd, void *dst, int n)
{
    uint8_t *p = (uint8_t *)dst;
    int done   = 0;
    while (done < n) {
        ssize_t r = read(fd, p + done, n - done);
        if (r < 0) return -1;
        if (r == 0) break;          /* EOF — return how much we got */
        done += (int)r;
    }
    return done;
}

/* skip n bytes by reading and discarding */
static int xskip(int fd, uint32_t n)
{
    uint8_t tmp[64];
    while (n > 0) {
        uint32_t want = n < sizeof(tmp) ? n : (uint32_t)sizeof(tmp);
        int r = xread(fd, tmp, (int)want);
        if (r <= 0) return -1;
        n -= (uint32_t)r;
    }
    return 0;
}

static uint16_t le16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t le32(const uint8_t *p) { return (uint32_t)(p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24)); }

static void mono_to_stereo(int16_t *buf, uint32_t frames)
{
    for (int32_t i = (int32_t)frames - 1; i >= 0; i--) {
        buf[i * 2 + 1] = buf[i];
        buf[i * 2]     = buf[i];
    }
}

static void usage(const char *argv0)
{
    printf("usage: %s <file.wav>\n", argv0);
}

int main(int argc, char *argv[])
{
    if (argc < 2) { usage(argv[0]); return 1; }

    int src = open(argv[1], O_RDONLY);
    if (src < 0) {
        printf("wav: cannot open '%s'\n", argv[1]);
        return 1;
    }

    if (xread(src, hdr_buf, 12) < 12) {
        printf("wav: '%s' too short\n", argv[1]);
        close(src); return 1;
    }
    if (memcmp(hdr_buf, "RIFF", 4) != 0 || memcmp(hdr_buf + 8, "WAVE", 4) != 0) {
        printf("wav: not a WAVE file (%.4s / %.4s)\n",
               (char *)hdr_buf, (char *)(hdr_buf + 8));
        close(src); return 1;
    }

    uint16_t channels = 0, bits = 0;
    uint32_t sample_rate = 0, data_size = 0;
    int found_fmt = 0, found_data = 0;

    while (!found_data) {
        if (xread(src, hdr_buf, 8) < 8) break;

        uint32_t chunk_size = le32(hdr_buf + 4);

        if (memcmp(hdr_buf, "fmt ", 4) == 0) {
            int to_read = chunk_size < 16 ? (int)chunk_size : 16;
            if (xread(src, hdr_buf, to_read) < to_read) break;
            if (le16(hdr_buf) != 1) {
                printf("wav: only PCM format supported\n");
                close(src); return 1;
            }
            channels    = le16(hdr_buf + 2);
            sample_rate = le32(hdr_buf + 4);
            bits        = le16(hdr_buf + 14);
            found_fmt   = 1;
            if (chunk_size > 16) xskip(src, chunk_size - 16);

        } else if (memcmp(hdr_buf, "data", 4) == 0) {
            data_size  = chunk_size;
            found_data = 1;

        } else {
            xskip(src, chunk_size + (chunk_size & 1));
        }
    }

    if (!found_fmt || !found_data) {
        printf("wav: missing fmt or data chunk\n");
        close(src); return 1;
    }
    if (bits != 16) {
        printf("wav: only 16-bit supported, got %u-bit\n", bits);
        close(src); return 1;
    }
    if (channels < 1 || channels > 2) {
        printf("wav: unsupported channel count %u\n", channels);
        close(src); return 1;
    }

    printf("wav: %s\n",     argv[1]);
    printf("     %u Hz  %u-bit  %s\n",
           sample_rate, bits, channels == 1 ? "mono" : "stereo");
    printf("     data: %u bytes\n", data_size);

    int audio = open(AUDIO_DEV, O_WRONLY);
    if (audio < 0) {
        printf("wav: cannot open " AUDIO_DEV "\n");
        close(src); return 1;
    }

    uint32_t src_frame  = channels == 1 ? 2u : 4u;
    uint32_t max_frames = (IO_BUF_SIZE / 2) / 2;
    uint32_t remaining  = data_size;
    uint32_t written    = 0;

    while (remaining > 0) {
        uint32_t want_bytes = max_frames * src_frame;
        if (want_bytes > remaining) want_bytes = remaining;

        int got = xread(src, pcm_buf, (int)want_bytes);
        if (got <= 0) break;

        uint32_t frames = (uint32_t)got / src_frame;
        if (frames == 0) break;

        if (channels == 1)
            mono_to_stereo(pcm_buf, frames);

        uint32_t out_bytes = frames * 4;
        uint32_t out_off   = 0;

        while (out_off < out_bytes) {
            ssize_t w = write(audio, (const uint8_t *)pcm_buf + out_off,
                              out_bytes - out_off);
            if (w < 0) {
                printf("wav: write error at %u bytes\n", written);
                goto done;
            }
            if (w == 0) {
                /* DMA ring full — yield the CPU before retrying so we don't
                 * burn cycles on back-to-back syscalls while the ring drains. */
                __asm__ volatile("pause");
            } else {
                out_off += (uint32_t)w;
                written += (uint32_t)w;
            }
        }
        remaining -= (uint32_t)got;
    }
done:
    printf("wav: done, %u bytes written\n", written);

    close(audio);
    close(src);
    return 0;
}