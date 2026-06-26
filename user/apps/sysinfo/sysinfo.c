/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: sysinfo.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include "libdesktop.h"
#include "exui.h"
#include "libfont.h"
#include "vad.h"
#include <emx/sinfo.h>

//-/////////////////////////////////////////-//
//-//                                     //-//
//-//               SYSINFO               //-//
//-//      a system information app       //-//
//-//                                     //-//
//-/////////////////////////////////////////-//

#define APP_TITLE  "system informations"
// will later jus be the min size of the window
#define APP_W      420
#define APP_H      340
#define PAD        5
#define FONT FONT8X12

#define LOGO_PATH "/emr/system/desktop/icons/logo.vad"
#define LOGO_W 120
#define LOGO_H 60

#define KBD_PATH "/dev/input/keyboard0"
// the content redraws so uptime, ram etc show correct inforamtions
#define UPDATE_INTERVAL 500000

// colors, all ARGB
#define SI_BG      0xFFD4D0C8u
#define SI_FG      0xFF000000u
#define SI_LABEL   0xFF000080u
#define SI_VALUE   0xFF333333u
#define SI_SEP     0xFF808080u
#define SI_TITLE   0xFF000080u

typedef struct {
    unsigned int keycode;
    unsigned int modifiers;
    unsigned char pressed;
} key_event_t;

// logo loaded once at startup
static vad_image_t g_logo;
static int g_has_logo = 0;
static int g_cw = 0;
static int g_ch = 0;

static long read_meminfo_kb(const char *key)
{
    FILE *f = fopen("/proc/meminfo", "r"); // first line
    if (!f) return -1;
    char line[128];
    size_t klen = strlen(key);
    long val = -1;
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, key, klen) == 0)
        {
            const char *p = line + klen;
            while (*p == ' ' || *p == ':') p++;
            val = 0;
            while (*p >= '0' && *p <= '9') { val = val * 10 + (*p - '0'); p++; }
            break;
        }
    }
    fclose(f);
    return val;
}

// it saves all processes from /proc and uses JUST the numbers to show the proces numb on the iformations
static int count_processes(void)
{
    DIR *d = opendir("/proc");
    if (!d) return -1;
    struct dirent *e;
    int count = 0;
    while ((e = readdir(d)))
    {
        const char *n = e->d_name;
        int is_num = (n[0] >= '1' && n[0] <= '9');

        for (int i = 1; n[i] && is_num; i++)if (n[i] < '0' || n[i] > '9') is_num = 0;
        if (is_num) count++;
    }
    closedir(d);
    return count;
}

// turns raw KB into something readable like "512 MB" or "2 GB (2048 MB)" but it doesnt round up or displays commas
static void fmt_mem(long kb, char *out, int sz)
{
    if (kb < 0)      { snprintf(out, sz, "n/a");         return; }
    if (kb < 1024)   { snprintf(out, sz, "%ld KB", kb);  return; }
    if (kb < 1048576){ snprintf(out, sz, "%ld MB", kb / 1024);return;}
    snprintf(out, sz, "%ld GB (%ld MB)", kb / 1048576, kb / 1024);
}

// formats uptime seconds into days/hours/minutes/seconds
static void fmt_uptime(unsigned long s, char *out, int sz)
{
    unsigned long d = s / 86400;
    unsigned long h = (s % 86400) / 3600;
    unsigned long m = (s % 3600) / 60;
    unsigned long sec = s % 60;
    if (d > 0) snprintf(out, sz, "%lud %02lu:%02lu:%02lu", d, h, m, sec);
    else snprintf(out, sz, "%02lu:%02lu:%02lu", h, m, sec);
}

// mb a divider comes into exui itself SOOO
// TODO:
// if a divider is IN exui THEN EMEX U FOOL UPDATE THIS SHIT!! :)))
static void hline(int x, int y, int w, unsigned int color)
{
    exui.DrawRectangle(x, y, w, 1, color);
}

// 2 sections, one on left and the other on the right, both display different labels
static void draw_row(
	int x, int *y, int content_w,
	const char *label, const char *value
){
    int fw = font_w(FONT);
    int fh = font_h(FONT);
    int label_x = x;
    int value_x = x + 110;

    exui.Print(label_x, *y, label, SI_LABEL, SI_BG, FONT);

    // if the value is too long just cut it off, if the de has window resizing this wont be a problem anymore
    // TODO:
    // window resizing
    int max_chars = (content_w - 110) / fw;
    char tmp[128];
    if (max_chars > 0 && max_chars < 127) {
        strncpy(tmp, value, max_chars);
        tmp[max_chars] = '\0';
    } else {
        strncpy(tmp, value, 127);
        tmp[127] = '\0';
    }
    exui.Print(value_x, *y, tmp, SI_VALUE, SI_BG, FONT8X12_BOLD);
    *y += fh + 3;
}

// reads all the live data and redraws the whole window
static void redraw(void)
{
    struct sysinfo_t 	kinfo;
    emx_sinfo_t emx;
    sysinfo(&kinfo);
    emx_sinfo(&emx);

    long mem_total_kb = read_meminfo_kb("MemTotal");
    long mem_free_kb = read_meminfo_kb("MemFree");
    long mem_used_kb = (mem_total_kb >= 0 && mem_free_kb >= 0)? mem_total_kb - mem_free_kb : -1;

    int proc_count = count_processes();

    char s_total[32];
    char s_used[32];
    char s_free[32];
    char s_uptime[32];
    char s_proc[16];
    char s_build[32];
    char s_arch[32];

    fmt_mem(mem_total_kb, s_total,  sizeof(s_total));
    fmt_mem(mem_used_kb,  s_used,   sizeof(s_used));
    fmt_mem(mem_free_kb,  s_free,   sizeof(s_free));

    fmt_uptime(kinfo.uptime, s_uptime, sizeof(s_uptime));

    snprintf(s_proc,  sizeof(s_proc),  "%d", proc_count);
    snprintf(s_build, sizeof(s_build), "%s", emx.version);
    snprintf(s_arch,  sizeof(s_arch),  "%s", emx.machine);

    exui.Clear(SI_BG);

    int y = PAD;

    // logo at the top if we have it, but the vad loader is still broken lol
    if (g_has_logo) {
        int lx = (g_cw - LOGO_W) / 2;
        if (lx < 0) lx = 0;
        vad_set_bg(SI_BG);
        vad_draw_scaled(&g_logo, lx, y, LOGO_W, LOGO_H);
        y += LOGO_H + 6;
    }

    // "emexOS" centered as the title
    {
        const char *title = "emexOS";
        int tw = (int)strlen(title) * font_w(FONT8X12_BOLD); // bold looks cooler
        int tx = (g_cw - tw) / 2;
        exui.Print(tx, y, title, SI_TITLE, SI_BG, FONT);
        y += font_h(FONT) + 2;

        // version + build as subtitle, also centered
        char subtitle[64];
        snprintf(subtitle, sizeof(subtitle), " -=== ver.%s, %s  ===- ", emx.release, s_build);
        int sw = (int)strlen(subtitle) * font_w(FONT);
        int sx = (g_cw - sw) / 2;
        exui.Print(sx, y, subtitle, SI_SEP, SI_BG, FONT);
        y += font_h(FONT) + 6;
    }

    hline(PAD, y, g_cw - PAD * 2, SI_SEP);
    y += 5;

    // general system stuff
    int row_x = PAD;
    draw_row(row_x, &y, g_cw - PAD, "     KERNEL",  emx.sysname);
    draw_row(row_x, &y, g_cw - PAD, "       ARCH",  s_arch);
    draw_row(row_x, &y, g_cw - PAD, "     UPTIME",  s_uptime);

    hline(PAD, y, g_cw - PAD * 2, SI_SEP);
    y += 5;

    // ram section, this is the main live stuff
    draw_row(row_x, &y, g_cw - PAD, "  TOTAL RAM", s_total);
    draw_row(row_x, &y, g_cw - PAD, "   USED RAM", s_used);
    draw_row(row_x, &y, g_cw - PAD, "   FREE RAM", s_free);

    hline(PAD, y, g_cw - PAD * 2, SI_SEP);
    y += 5;

    // process count, also live
    draw_row(row_x, &y, g_cw - PAD, "  PROCESSES", s_proc);

    exui.Flush();
}

// spins for a bit without fully blocking
static void soft_sleep(void)
{
    for (int i = 0; i < UPDATE_INTERVAL; i++)
        __asm__ volatile("pause");
}

int main(void)
{
    // keyboard is optional mb later for options idk
    // mb this will get a system settings app lol
    int g_kbd = open(KBD_PATH, O_RDONLY);

    // center the window on 1280x720
    int scr_w = 1280, scr_h = 720;
    int win_x = (scr_w - APP_W) / 2;
    int win_y = (scr_h - APP_H) / 2;
    if (win_x < 0) win_x = 0;
    if (win_y < 0) win_y = 0;

    desktop.createWindow(APP_TITLE, win_x, win_y, APP_W, APP_H, DT_WIN);
    DesktopArea ca = desktopContentArea(win_x, win_y, APP_W, APP_H, DT_WIN);
    exui_init(ca.w, ca.h);

    g_cw = ca.w;
    g_ch = ca.h;

    // the buffer exists after exui_init but it doesnt work cuz of libvad
    g_has_logo = (vad_load(LOGO_PATH, &g_logo) == 0);
    printf("debug: logo load %s\n", g_has_logo ? "ok" : "failed");

    for (;;)
    {
        redraw();

        if (g_kbd >= 0)
        {
            key_event_t ev;
            int r = (int)read(g_kbd, &ev, sizeof(ev));

            if (r == (int)sizeof(ev) && ev.pressed)
            {
                if (ev.keycode == 0x1B) break;
            }
        }

        soft_sleep();
    }

    if (g_has_logo) vad_free(&g_logo);
    if (g_kbd >= 0) close(g_kbd);

    desktop.closeWindow();

    return 0;
}