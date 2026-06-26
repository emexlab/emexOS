/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: vt.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef DEVICE_VT_H
#define DEVICE_VT_H

#include <types.h>
#include <kernel/module/module.h>

/* same layout as /dev/input/keyboard0 */
typedef struct {
    u32     keycode;
    u32     modifiers;

    u8  pressed;
    u8  _pad[3];
} vt_key_event_t;

#define VT_INPUT_BUF_SIZE    4096
#define VT_OUTPUT_BUF_SIZE   65536


#define VT_STATE_FREE     0
#define VT_STATE_ACTIVE   1
#define VT_STATE_FOCUSED  2  /* gets keyboard input */

#define VT_MAX 16

typedef struct
{
    u64     id;      /* never reused */
    int     state;

    u8     in_buf[VT_INPUT_BUF_SIZE];
    u32     in_head;
    u32     in_tail;
    u32     in_count;

    u8     out_buf[VT_OUTPUT_BUF_SIZE];
    u32     out_head;
    u32     out_tail;
    u32     out_count;

    void *input_node;
    void *output_node;
} vt_t;

/* ioctl for controll*/
#define VT_CREATE      0x5600
#define VT_DESTROY     0x5601
#define VT_FOCUS       0x5602
#define VT_GETFOC      0x5603

void vt_subsystem_init(void);
void vt_destroy(u64 id);
void vt_focus(u64 id);

u64 vt_get_focused(void);
u64 vt_create(void);

vt_t *vt_get(u64 id);

int vt_output_write (vt_t *vt, const void *buf, size_t count);
int vt_output_read  (vt_t *vt, void *buf, size_t count);
int vt_input_write  (vt_t *vt, const vt_key_event_t *ev);

/* blocks til data*/
int vt_input_read(vt_t *vt, void *buf, size_t count);

int vt_ctrl_ioctl(int request, void *arg);

extern driver_module vt_module;

#endif