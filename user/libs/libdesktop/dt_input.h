/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: dt_input.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#define DT_EV_NONE  0
#define DT_EV_MOUSE 1
#define DT_EV_KEY   2

#define DT_BTN_LEFT   	0x01
#define DT_BTN_RIGHT  	0x02
#define DT_BTN_MIDDLE 	0x04

typedef struct
{
    unsigned char 	type;
    unsigned char 	buttons;
    unsigned char 	modifiers;
    unsigned char 	pressed;
    unsigned int  	keycode;

    short mx, my;
    signed char   scroll;

} dt_event_t;

#define DT_EVENT_QUEUE_MAX 64
#define DT_INPUT_PREFIX "/tmp/dt/input_"