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

#pragma once

#include <kernel/devices/disks/hdd0.h>
#include <kernel/devices/fb0/fb0.h>
#include <kernel/devices/input/kbd.h>
#include <kernel/devices/input/mouse0.h>
#include <kernel/devices/net/eth0.h>
#include <kernel/devices/null/null.h>
#include <kernel/devices/random/random.h>
#include <kernel/devices/random/urandom.h>
#include <kernel/devices/zero/zero.h>
#include <kernel/devices/audio/audio0.h>
#include <kernel/devices/vt/vt.h>

/*
 * version numbers probably dont get updated, so dont wonder btw xd
 */

#define HDANAME "dev_hda0"
#define HDAPATH "/dev/hda0"
#define HDAUNIVERSAL VERSION_NUM(0, 1, 2, 0)

#define KBDNAME "dev_ps2_keyboard0"
#define KBDPATH "/dev/input/keyboard0"
#define KBDUNIVERSAL VERSION_NUM(0, 3, 1, 0)

#define MS0NAME "dev_ps2_mouse0"
#define MS0PATH "/dev/input/mouse0"
#define MS0UNIVERSAL VERSION_NUM(0, 0, 0, 0)

#define ETH0NAME "dev_eth0"
#define ETH0PATH "/dev/net/eth0"
#define ETH0UNIVERSAL VERSION_NUM(0, 1, 0, 0)

#define EFBNAME FBN
#define FB0NAME "dev_fb0"
#define FB0PATH "/dev/fb0"
#define FB0UNIVERSAL VERSION_NUM(0, 0, 0, 0) // always 0.0.0.0

#define VTNAME "dev_vt"
#define VTPATH "/dev/vt"
#define VTUNIVERSAL VERSION_NUM(0, 1, 0, 0)

#define ZERNAME "dev_zero"
#define ZERPATH "/dev/zero"
#define ZERUNIVERSAL VERSION_NUM(0, 0, 0, 0) // always 0.0.0.0

#define NULNAME "dev_null"
#define NULPATH "/dev/null"
#define NULUNIVERSAL VERSION_NUM(0, 0, 0, 0) // always 0.0.0.0

#define URNDNAME "dev_urandom"
#define URNDPATH "/dev/urandom"
#define URNDUNIVERSAL VERSION_NUM(0, 0, 1, 0)

#define RNDNAME "dev_random"
#define RNDPATH "/dev/random"
#define RNDUNIVERSAL VERSION_NUM(0, 0, 1, 0)

#define AUDIO0NAME    "dev_audio0"
#define AUDIO0PATH    "/dev/audio0"
#define AUDIO0UNIVERSAL VERSION_NUM(0, 1, 0, 0)
