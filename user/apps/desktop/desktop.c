/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: desktop.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "config/cfg.h"
#include "compositor/comp.h"
#include "bg/bg.h"
#include "win/win.h"
#include "render/render.h"
#include "cursor/cursor.h"
#include "input/input.h"
#include "taskbar/taskbar.h"
#include "cmd/cmd.h"

#include <emx/fb.h>

#define DT_WBUF_PREFIX "/tmp/dt/wbuf_"

extern drag_info_t g_input_drag_prev;

static int _slen(const char *s)
{
	int n = 0;
	while (s[n]) n++;
	return n;
}

static void _itoa(int v, char *out)
{
    char tmp[16];
    int i = 0;
    int neg = (v < 0);
    int j = 0;

    if (v == 0)
    {
    	out[0]='0';
     	out[1]='\0';
     	return;
    }

    if (neg) v = -v;
    while (v)
    {
    	tmp[i++] = '0' + v % 10;
        v /= 10;
    }
    if (neg) tmp[i++] = '-';
    while (i > 0) {
        out[j++] = tmp[--i];
    }
    out[j] = '\0';
}

static void write_cursor_pos(int x, int y)
{
    char buf[32];
    char xs[12];
    char ys[12];
    int i = 0;
    int j = 0;
    int fd = open(DT_CURSOR, O_WRONLY | O_CREAT);

    _itoa(x, xs);
    _itoa(y, ys);

    while (xs[j]) buf[i++] = xs[j++];
    buf[i++] = ' ';
    j = 0;

    while (ys[j]) buf[i++] = ys[j++];

    buf[i++] = '\n';
    buf[i] = '\0';

    if (fd >= 0) { write(fd, buf, (unsigned)_slen(buf)); close(fd); }
}

static void current_content(
    dt_win_t *w,
    int *cx, int *cy, int *cw, int *ch
) {
    unsigned int s = w->style;
    if (s & DT_POPUP) {
        *cx = w->x + 1;
        *cy = w->y + 1;
        *cw = w->w - 2;
        *ch = w->h - 2;
    } else if (s & DT_NOTITLE) {
        *cx = w->x + DT_BORDER;
        *cy = w->y + DT_BORDER;
        *cw = w->w - DT_BORDER * 2;
        *ch = w->h - DT_BORDER * 2;
    } else {
        *cx = w->x + DT_BORDER;
        *cy = w->y + DT_TITLE_H + 1;
        *cw = w->w - DT_BORDER * 2;
        *ch = w->h - DT_TITLE_H - 1 - DT_BORDER;
    }
}

static void _old_full_rect(
    dt_win_t *w, int hcx, int hcy, int hcw, int hch,
    int *ox, int *oy, int *ow, int *oh
){
    unsigned int s = w->style;
    if (s & DT_POPUP) {
        *ox = hcx - 1;
        *oy = hcy - 1;
        *ow = hcw + 2;
        *oh = hch + 2;
    } else if (s & DT_NOTITLE) {
        *ox = hcx - DT_BORDER;
        *oy = hcy - DT_BORDER;
        *ow = hcw + DT_BORDER * 2;
        *oh = hch + DT_BORDER * 2;
    } else {
        *ox = hcx - DT_BORDER;
        *oy = hcy - DT_TITLE_H - 1;
        *ow = hcw + DT_BORDER * 2;
        *oh = hch + DT_TITLE_H + 1 + DT_BORDER;
    }
}

static void sync_home_to_current(void)
{
    int i;
    for (i = 0; i < DT_WIN_MAX; i++)
    {
        dt_win_t *w = win_get(i);
        if (!w) continue;

        int cx, cy, cw, ch;
        current_content(w, &cx, &cy, &cw, &ch);

        // if window hasn't moved or resized, nothing to do
        if (cx == w->home_cx && cy == w->home_cy && cw == w->home_cw && ch == w->home_ch) continue;

        int ox, oy, ow, oh;
        _old_full_rect(
            w, w->home_cx, w->home_cy, w->home_cw, w->home_ch,
            &ox, &oy, &ow, &oh
        );
        int nox, noy, now_, noh;
        _old_full_rect(w, cx, cy, cw, ch, &nox, &noy, &now_, &noh);
        {
            int ix0 = ox > nox ? ox : nox;
            int iy0 = oy > noy ? oy : noy;
            int ix1 = (ox+ow)<(nox+now_) ? (ox+ow) : (nox+now_);
            int iy1 = (oy+oh)<(noy+noh) ? (oy+oh) : (noy+noh);
            if (ix0 >= ix1 || iy0 >= iy1) {
                bg_draw_rect(ox, oy, ow, oh);
            } else {
                if (oy < iy0)
                    bg_draw_rect(ox, oy, ow, iy0-oy);
                if (oy+oh > iy1)
                    bg_draw_rect(ox, iy1, ow, (oy+oh)-iy1);
                if (iy0 < iy1) {
                    if (ox < ix0)
                        bg_draw_rect(ox, iy0, ix0-ox, iy1-iy0);
                    if (ox+ow > ix1)
                        bg_draw_rect(ix1, iy0, (ox+ow)-ix1, iy1-iy0);
                }
            }
        }

        w->home_cx = cx;
        w->home_cy = cy;
        w->home_cw = cw;
        w->home_ch = ch;
    }
}

static void clear_closed_windows(void)
{
    if (!g_input_drag_prev.valid) return;
    bg_draw_rect(
        g_input_drag_prev.wx,
        g_input_drag_prev.wy,
        g_input_drag_prev.ww,
        g_input_drag_prev.wh
    );
    g_input_drag_prev.valid = 0;
}

static void clear_cmd_rects(cmd_result_t *cr)
{
    int i;
    for (i = 0; i < cr->count; i++)
    {
        drag_info_t *d = &cr->rects[i];
        if (d->valid) bg_draw_rect(d->wx, d->wy, d->ww, d->wh);
    }
}

static void _build_wbuf_path(char *out, pid_t pid)
{
    const char *pfx = DT_WBUF_PREFIX;
    int i = 0;
    while (*pfx) out[i++] = *pfx++;
    char tmp[12];
    int ti = 0;
    unsigned int p = (unsigned int)pid;
    if (!p) { tmp[ti++] = '0'; }
    else { while (p) { tmp[ti++] = '0' + p % 10; p /= 10; } }
    while (ti > 0) out[i++] = tmp[--ti];
    out[i] = '\0';
}

static int _read_exact_d(int fd, void *buf, int n)
{
    unsigned char *p = (unsigned char *)buf;
    int rem = n;
    while (rem > 0)
    {
        int r = (int)read(fd, p, (unsigned)rem);
        if (r <= 0) return -1;
        p += r;
        rem -= r;
    }
    return 0;
}

static void refresh_win_bufs(void)
{
    char path[64];
    int header[2];
    int i;

    for (i = 0; i < DT_WIN_MAX; i++) {
        dt_win_t *wn = win_get(i);
        if (!wn) continue;

        _build_wbuf_path(path, wn->pid);

        int fd = open(path, O_RDONLY);
        if (fd < 0) continue;

        if (_read_exact_d(fd, header, 8) != 0) { close(fd); continue; }

        int bw = header[0], bh = header[1];
        if (bw <= 0 || bh <= 0 || bw > 4096 || bh > 4096) { close(fd); continue; }

        // reallocate buffer if size changed
        if (wn->buf && (wn->buf_w != bw || wn->buf_h != bh)) {
            free(wn->buf);
            wn->buf = (unsigned int *)malloc((unsigned)(bw * bh * 4));
            if (!wn->buf) { wn->buf_w = 0; wn->buf_h = 0; close(fd); continue; }
            wn->buf_w = bw;
            wn->buf_h = bh;
        }
        // allocate fresh
        if (!wn->buf) {
            wn->buf = (unsigned int *)malloc((unsigned)(bw * bh * 4));
            if (!wn->buf) { close(fd); continue; }
            wn->buf_w = bw;
            wn->buf_h = bh;
        }

        // read directly into existing buffer, skip on size mismatch
        if (wn->buf_w == bw && wn->buf_h == bh) _read_exact_d(fd, wn->buf, bw * bh * 4);

        close(fd);
    }
}

static void render_band(input_state_t *is)
{
	/*
	 * RUBBERBAND FIX
		* by @offihito
	 */
	if (!is->sel_active) return;

	// compute actual rect (supports all drag directions)
	int x0 = is->sel_x0 < is->sel_x1 ? is->sel_x0 : is->sel_x1;
	int y0 = is->sel_y0 < is->sel_y1 ? is->sel_y0 : is->sel_y1;
	int x1 = is->sel_x0 > is->sel_x1 ? is->sel_x0 : is->sel_x1;
	int y1 = is->sel_y0 > is->sel_y1 ? is->sel_y0 : is->sel_y1;
	int w = x1 - x0;
	int h = y1 - y0;

	if (w < 2 || h < 2) return;

	int sw = comp_w();
	int sh = comp_h();

	for (int ry = y0 + 1; ry < y1 && ry < sh; ry++)
	{
		for (int rx = x0 + 1; rx < x1 && rx < sw; rx++)
		{
			unsigned int bg = comp_get(rx, ry);
			unsigned int br = (bg >> 16) & 0xFF;
			unsigned int bg2 = (bg >> 8) & 0xFF;
			unsigned int bb = bg & 0xFF;

			br = (br * 4 + 128) / 5;
			bg2 = (bg2 * 4 + 128) / 5;
			bb = (bb * 4 + 128) / 5;

			comp_set(rx, ry, 0xFF000000u | (br << 16) | (bg2 << 8) | bb);
		}
	}

	// top and bottom edges
	for (int rx = x0; rx <= x1 && rx < sw; rx++) {
		if (y0 >= 0 && y0 < sh) comp_set(rx, y0, BAND_BORDER);
		if (y1 >= 0 && y1 < sh) comp_set(rx, y1, BAND_BORDER);
	}
	// left and right edges
	for (int ry = y0; ry <= y1 && ry < sh; ry++) {
		if (x0 >= 0 && x0 < sw) comp_set(x0, ry, BAND_BORDER);
		if (x1 >= 0 && x1 < sw) comp_set(x1, ry, BAND_BORDER);
	}
}

int main(void)
{
    mkdir(DT_DIR);

    int fd;
    fd = open(DT_CMD, O_WRONLY | O_CREAT); if (fd >= 0) close(fd);
    fd = open(DT_DIRTY, O_WRONLY | O_CREAT);
    if (fd >= 0) close(fd);

    int fb = open("/dev/fb0", O_RDWR);
    int mfd = open(MOUSE_DEV, O_RDONLY);

    if (fb < 0 || mfd < 0) return 1;

    fb_var_screeninfo vinfo;
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);

    int scr_w = (int)vinfo.xres;
    int scr_h = (int)vinfo.yres;

    //TODO:
    // look for real display size
    if (scr_w <= 0) scr_w = 1024;
    if (scr_h <= 0) scr_h = 768;

    comp_init(fb, scr_w, scr_h);
    bg_init(scr_w, scr_h);
    cur_init(fb, scr_w, scr_h);
    taskbar_init(scr_w, scr_h);
    input_init();

    bg_draw_full();
    comp_flush();

    input_state_t is;
    is.cx = scr_w / 2;
    is.cy = scr_h / 2;

    is.win_changed = 0;
    is.sel_active = 0;
    is._sel_was_active = 0;
    is.sel_x0 = 0; is.sel_y0 = 0;
    is.sel_x1 = 0; is.sel_y1 = 0;
    is.sel_px1 = 0; is.sel_py1 = 0;

    cur_draw_fb(is.cx, is.cy);
    write_cursor_pos(is.cx, is.cy);

    int poll_tick = 0;

    #define POLL_INTERVAL 4

    for (;;)
    {
        int need_full = 0;
        int need_cur  = 0;

        cmd_result_t cr;
        cr.count = 0;
        cr.win_changed = 0;

        poll_tick++;

        int app_dirty = 0;
        if (poll_tick >= POLL_INTERVAL)
        {
            poll_tick = 0;
            cmd_process(&cr);
            if (cr.win_changed) need_full = 1;
            app_dirty = cmd_check_dirty();
            if (app_dirty) need_full = 1;
        }

        input_frame_begin(&is);

        if (input_drain(mfd, &is))
        {
            need_cur = 1;
            write_cursor_pos(is.cx, is.cy);
        }
        if (is.win_changed)
        {
            need_full = 1;
            is.win_changed = 0;
        }

        int was_active = is._sel_was_active;
        is._sel_was_active = is.sel_active;

        if (is.sel_active || was_active) need_full = 1;

        if (need_full)
        {
            refresh_win_bufs();
            comp_capture();
            cur_undo_from_backbuf();

            // rubberband fix; @offihito
            if (was_active)
            {
              int x0 = is.sel_x0 < is.sel_px1 ? is.sel_x0 : is.sel_px1;
              int y0 = is.sel_y0 < is.sel_py1 ? is.sel_y0 : is.sel_py1;
              int x1 = is.sel_x0 > is.sel_px1 ? is.sel_x0 : is.sel_px1;
              int y1 = is.sel_y0 > is.sel_py1 ? is.sel_y0 : is.sel_py1;
              bg_draw_rect(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
            }

            sync_home_to_current();
            clear_cmd_rects(&cr);
            clear_closed_windows();
            render_band(&is); // under wins
            render_all();
            taskbar_draw(is.cx, is.cy, 0);
            cur_bake(is.cx, is.cy);
            comp_flush();
        } else if (need_cur)
        {
            cur_erase_fb();
            cur_draw_fb(is.cx, is.cy);
        }

        if (!need_full && !need_cur)
        {
            for (volatile int i = 0; i < 500; i++)
                __asm__ volatile("pause");
        }
    }
}