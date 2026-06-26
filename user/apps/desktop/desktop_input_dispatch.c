/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: desktop_input_dispatch.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "desktop_input_dispatch.h"

#include <unistd.h>
#include <fcntl.h>

#define DT_INPUT_PFX "/tmp/dt/input_"

static int g_scr_w = 1280;
static int g_scr_h = 720;

void dt_set_screen_size(int w, int h)
{
    g_scr_w = w;
    g_scr_h = h;
}

static void _itoa(int v, char *out)
{
    char tmp[16];
    int i=0;
    int j=0;

    if (v == 0) {
    	out[0] = '0';
     	out[1] = '\0';
      	return;
    }
    while (v > 0) {
    	tmp[i++] = '0' + v % 10;
     	v /= 10;
    }
    while (i > 0) out[j++] = tmp[--i];

    out[j]='\0';
}

static void _pid_path(pid_t pid, char *out)
{
    const char *pfx=DT_INPUT_PFX;

    int i=0;
    int j=0;
    char ps[12];

    while (*pfx) out[i++]=*pfx++;

    _itoa((int)pid, ps);

    while (ps[j]) out[i++]=ps[j++];
    out[i]='\0';
}

void dt_dispatch_event(pid_t focused_pid, const dt_event_t *ev)
{
    if (focused_pid<=0||!ev) return;

    char path[64]; _pid_path(focused_pid, path);

    // read existing events first so we can append
    static unsigned char existing[sizeof(dt_event_t)*DT_EVENT_QUEUE_MAX];

    int elen=0;
    int fd = open(path, O_RDONLY);

    if (fd >= 0)
    {
        int r = (int)read(fd, existing, sizeof(existing));
        close(fd);
        if (r > 0) elen = r;
    }

    // drop oldest if queue full
    int max_bytes= (int)(sizeof(dt_event_t) *DT_EVENT_QUEUE_MAX );
    if (elen + (int)sizeof(dt_event_t) > max_bytes) return;

    fd = open(path, O_WRONLY|O_CREAT);
    if (fd < 0) return;
    if (elen > 0) write(fd, existing, (unsigned)elen);
    write(fd, ev, sizeof(dt_event_t));

    close(fd);
}

void dt_clear_input(pid_t pid)
{
    if (pid<=0) return;

    char path[64]; _pid_path(pid, path);
    int fd=open(path, O_WRONLY|O_CREAT);

    if (fd>=0) close(fd);
}

int dt_make_mouse_event(
	int focused_idx, int abs_x, int abs_y,
    unsigned char buttons, dt_event_t *out
){
    dt_win_t *w=win_get(focused_idx);

    if (!w) return 0;

    out->type     = DT_EV_MOUSE;
    out->mx       = (short)(abs_x - w->home_cx);
    out->my       = (short)(abs_y - w->home_cy);
    out->buttons  = buttons;
    out->scroll   = 0;
    out->keycode  = 0;
    out->pressed  = 0;
    out->modifiers= 0;

    return 1;
}

int dt_make_key_event(
	unsigned int keycode, unsigned char modifiers,
    unsigned char pressed, dt_event_t *out
){
    out->type     = DT_EV_KEY;
    out->keycode  = keycode;
    out->modifiers= modifiers;
    out->pressed  = pressed;
    out->mx=0; out->my=0; out->buttons=0; out->scroll=0;
    return 1;
}

// popup clamping
void dt_clamp_popup(int *x, int *y, int w, int h)
{
    if (*x+w > g_scr_w) *x = g_scr_w-w;
    if (*y+h > g_scr_h) *y = g_scr_h-h;
    if (*x < 0) *x=0;
    if (*y < 0) *y=0;
}