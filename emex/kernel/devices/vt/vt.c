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

#include "vt.h"

#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/mem/klime/klime.h>
#include <kernel/mem/lib/main.h>
#include <kernel/communication/serial.h>
#include <kernel/module/module.h>
#include <kernel/graph/lib/string.h>

#include <types.h>

static vt_t vt_table[VT_MAX];
static u64  vt_id_counter = 0;
static u64  vt_focused    = (u64)-1;
static int  vt_inited     = 0;

extern void *fs_klime;

static int ring_write(
	u8 *buf, u32 *head, u32 *tail,
    u32 *count, u32 cap,
    const void *src, size_t n
){
    (void)head;
    const u8 *s = (const u8 *)src;
    size_t w = 0;

    while (w < n && *count < cap)
    {
        buf[*tail] = s[w++];
        *tail = (*tail + 1) % cap;
        (*count)++;
    }

    return (int)w;
}

static int ring_read(
	u8 *buf, u32 *head, u32 *tail,
    u32 *count, u32 cap,
    void *dst, size_t n
){
    (void)tail;
    u8 *d = (u8 *)dst;
    size_t r = 0;

    while (r < n && *count > 0)
    {
        d[r++] = buf[*head];
        *head = (*head + 1) % cap;
        (*count)--;
    }

    return (int)r;
}

vt_t *vt_get(u64 id)
{
    if (!vt_inited) return NULL;

    for (int i = 0; i < VT_MAX; i++)
    {
        if (
        	vt_table[i].state != VT_STATE_FREE && vt_table[i].id == id
        ) return &vt_table[i];
    }
    return NULL;
}

static u64 id_from_path(const char *path)
{
    const char *p = path;
    int slash = 0;

    while (*p)
    {
        if (*p == '/') { slash++; p++; continue; }
        if (slash == 2)
        {
            u64 id = 0;
            while (*p >= '0' && *p <= '9') { id = id * 10 + (u64)(*p - '0'); p++; }
            return id;
        }

        p++;
    }

    return (u64)-1;
}

static void *vt_output_open(const char *path)
{
    u64 id = id_from_path(path);
    if (!vt_get(id)) return NULL;
    return (void *)(uintptr_t)(id + 1);
}

static int vt_output_write_cb(void *handle, const void *buf, size_t count, u64 offset)
{
    (void)offset;

    vt_t *vt = vt_get((u64)(uintptr_t)handle - 1);
    if (!vt) return -1;
    return vt_output_write(vt, buf, count);
}

static int vt_output_read_cb(void *handle, void *buf, size_t count, u64 offset)
{
    (void)offset;

    vt_t *vt = vt_get((u64)(uintptr_t)handle - 1);
    if (!vt) return -1;

    return vt_output_read(vt, buf, count);
}

static void *vt_input_open(const char *path)
{
    u64 id = id_from_path(path);
    if (!vt_get(id)) return NULL;

    return (void *)(uintptr_t)(id + 1);
}

/* terminal app pushes vt_key_event_t structs */
static int vt_input_write_cb(void *handle, const void *buf, size_t count, u64 offset)
{
    (void)offset;

    vt_t *vt = vt_get((u64)(uintptr_t)handle - 1);
    if (!vt) return -1;

    const u8 *src = (const u8 *)buf;

    size_t ev_size = sizeof(vt_key_event_t);
    int done = 0;

    while ((size_t)done + ev_size <= count)
    {
        vt_key_event_t ev;
        for (size_t i = 0; i < ev_size; i++) ((u8 *)&ev)[i] = src[done + i];
        done += (int)ev_size;
        if (ev.pressed) vt_input_write(vt, &ev);
    }

    return done;
}

static int vt_input_read_cb(void *handle, void *buf, size_t count, u64 offset)
{
    (void)offset;

    vt_t *vt = vt_get((u64)(uintptr_t)handle - 1);
    if (!vt) return -1;

    return vt_input_read(vt, buf, count);
}

/* one heap-allocated slot per vt node */
typedef struct {
    driver_module mod;
    char 	name_buf[32];
    char 	path_buf[48];
} vt_mod_slot_t;

static void append_u64(char *s, u64 v)
{
    char tmp[24];
    int i = 0;

    if (!v) { tmp[i++] = '0'; }
    else { while (v) { tmp[i++] = '0' + (int)(v % 10); v /= 10; } }

    int len = i;
    int j = str_len(s);

    for (int a = 0, b = len - 1; a < b; a++, b--) { char c = tmp[a]; tmp[a] = tmp[b]; tmp[b] = c; }
    for (int k = 0; k < len; k++) s[j + k] = tmp[k];

    s[j + len] = '\0';
}

static driver_module *make_vt_module(u64 id, int is_output)
{
    vt_mod_slot_t *slot = (vt_mod_slot_t *)klime_create(
    	(klime_t *)fs_klime, sizeof(vt_mod_slot_t)
    );

    if (!slot) return NULL;

    str_copy(slot->name_buf, is_output ? "dev_vt_out_" : "dev_vt_in_");
    append_u64(slot->name_buf, id);

    str_copy(slot->path_buf, "/dev/vt/");
    append_u64(slot->path_buf, id);
    str_append(slot->path_buf, is_output ? "/output" : "/input");

    driver_module *m = &slot->mod;
    m->name    = slot->name_buf;
    m->mount   = slot->path_buf;
    m->version = VERSION_NUM(0, 1, 0, 0);
    m->init    = NULL;
    m->fini    = NULL;

    if (is_output) {
        m->open  = vt_output_open;
        m->read  = vt_output_read_cb;
        m->write = vt_output_write_cb;
    } else {
        m->open  = vt_input_open;
        m->read  = vt_input_read_cb;
        m->write = vt_input_write_cb;
    }
    return m;
}

void vt_subsystem_init(void)
{
    if (vt_inited) return;

    for (int i = 0; i < VT_MAX; i++)
    {
        vt_table[i].state 	= VT_STATE_FREE;
        vt_table[i].id    	= 0;
    }

    vt_id_counter 	= 0;
    vt_focused    	= (u64)-1;
    vt_inited     	= 1;

    log("[VT]", "init\n", d);
}

u64 vt_create(void)
{
    if (!vt_inited) vt_subsystem_init();

    int slot = -1;

    for (int i = 0; i < VT_MAX; i++)
    {
        if (vt_table[i].state == VT_STATE_FREE) { slot = i; break; }
    }
    if (slot < 0) { log("[VT]", "no free slots\n", error); return (u64)-1; }

    u64 id = vt_id_counter++;
    vt_t *vt = &vt_table[slot];

    vt->id = id;
    vt->state = VT_STATE_ACTIVE;

    vt->in_head = vt->in_tail = vt->in_count = 0;
    vt->out_head = vt->out_tail = vt->out_count = 0;

    vt->input_node   = NULL;
    vt->output_node  = NULL;

    driver_module *in_mod    = make_vt_module(id, 0);
    driver_module *out_mod   = make_vt_module(id, 1);

    if (in_mod)module_register(in_mod);
    if (out_mod) module_register(out_mod);

    /* first vt gets focus */
    if (vt_focused == (u64)-1)
    {
        vt->state      = VT_STATE_FOCUSED;
        vt_focused     = id;
    }

    return id;
}

void vt_destroy(u64 id)
{
    vt_t *vt = vt_get(id);
    if (!vt) return;
    if (vt_focused == id) vt_focused = (u64)-1;
    vt->state = VT_STATE_FREE;
}

void vt_focus(u64 id)
{
    if (vt_focused != (u64)-1)
    {
        vt_t *old = vt_get(vt_focused);
        if (old && old->state == VT_STATE_FOCUSED)old->state = VT_STATE_ACTIVE;
    }

    vt_focused = (u64)-1;

    if (id == (u64)-1) return;
    vt_t *vt = vt_get(id);
    if (!vt) return;

    vt->state = VT_STATE_FOCUSED;
    vt_focused = id;
}

u64 vt_get_focused(void)
{
    return vt_focused;
}

int vt_output_write(vt_t *vt, const void *buf, size_t count)
{
    if (!vt) return -1;

    return ring_write(
    	vt->out_buf, &vt->out_head, &vt->out_tail,
        &vt->out_count, VT_OUTPUT_BUF_SIZE, buf, count
    );
}

int vt_output_read(vt_t *vt, void *buf, size_t count)
{
    if (!vt) return -1;

    return ring_read(
    	vt->out_buf, &vt->out_head, &vt->out_tail,
        &vt->out_count, VT_OUTPUT_BUF_SIZE, buf, count
    );
}

int vt_input_write(vt_t *vt, const vt_key_event_t *ev)
{
    if (!vt || !ev) return -1;

    return ring_write(
    	vt->in_buf, &vt->in_head, &vt->in_tail,
        &vt->in_count, VT_INPUT_BUF_SIZE, ev, sizeof(vt_key_event_t)
    );
}

int vt_input_read(vt_t *vt, void *buf, size_t count)
{
    if (!vt || !buf || !count) return -1;

    size_t ev_size = sizeof(vt_key_event_t);

    u8 *out  = (u8 *)buf;
    size_t produced = 0;

    while (produced < count)
    {
        /* wait for a full event */
        while (vt->in_count < ev_size)
        {
            __asm__ volatile("sti");
            __asm__ volatile("hlt");
        }

        vt_key_event_t ev;

        ring_read(
        	vt->in_buf, &vt->in_head, &vt->in_tail,
            &vt->in_count, VT_INPUT_BUF_SIZE, &ev, ev_size
        );

        if (!ev.pressed)continue;

        u32 kc = ev.keycode;
        char c = (char)(kc & 0xFF);

        if (kc == '\r' || kc == '\n') { out[produced++] = '\n'; break; }
        if (kc == '\b' || kc == 0x7F) { out[produced++] = '\b'; break; }
        if (c >= 0x20 && c <= 0x7E)   { out[produced++] = (u8)c; continue; }

        /* non-printable is skip */
    }

    return (int)produced;
}

/* /dev/vt/control */

int vt_ctrl_ioctl(int request, void *arg)
{
    if (!arg) return -1;

    switch ((unsigned int)request)
    {
        case VT_CREATE: {
            u64 id = vt_create();
            if (id == (u64)-1) return -1;
            *(u64 *)arg = id;
            return 0;
        }
        case VT_DESTROY: {
            vt_destroy(*(u64 *)arg);
            return 0;
        }
        case VT_FOCUS: {
            vt_focus(*(u64 *)arg);
            return 0;
        }
        case VT_GETFOC: {
            *(u64 *)arg = vt_get_focused();
            return 0;
        }
        default:
            return -1;
    }
}

static int ctrl_init(void)
{
    log("[VT]", "init /dev/vt/control\n", d);
    return 0;
}

static void *ctrl_open(const char *path)
{
	(void)path;
	return (void *)1;
}

static int ctrl_read(void *h, void *b, size_t n, u64 o)
{
	(void)h;
	(void)b;
	(void)n;
	(void)o;
	return 0;
}

static int ctrl_write(void *h, const void *b, size_t n, u64 o)
{
	(void)h;
	(void)b;
	(void)n;
	(void)o;
	return 0;
}

driver_module vt_module =
{
    .name    = "dev_vt",
    .mount   = "/dev/vt/control",
    .version = VERSION_NUM(0, 1, 0, 0),
    .init    = ctrl_init,
    .fini    = NULL,
    .open    = ctrl_open,
    .read    = ctrl_read,
    .write   = ctrl_write,
};
