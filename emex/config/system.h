/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: system.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */
#pragma once

#include "../ebuild.h"
#include "../emex.h"

#define KERNEL_ARCH "[x86-64]"
#define KERNEL_NAME __EMEX_KERNEL
#define KERNEL_RELEASE_VER __EMEX_VERSION_V
#define KERNEL_RELEASE_BUILD "" ___EMEX_BUILD " "
#define KERNEL_FULL KERNEL_BARENAME " " KERNEL_ARCH //" " KERNEL_DEFRELEASE
//#define VERIFYSYSGEN "62522870358368638010"
#define KERNELADRESSNUM "kman"
#define KERNELPROC "kernel"
#define KERNELSPACE 0x40000000
#define KERNELPRIORITY 255

#define JUMPTOUSER 1

#define USE_HCF 1
#define BOOTUP_VISUALS 0 // verbose boot == 0, silent boot == 1
#define BS_DEBUG 0 // verbose boot == 1, animation/gfx boot == 0
#define TTYNOGUI 1 // GUI == 0, no GUI == 1
#define DEBUG_LOGGING 1 // 1 on; 0 off

// 1 == run tests like processes, scheduler which are in early developement and not finished
// 0 == disable running those tests
#define RUNTESTS 1


// 1 == Hardware compatibility on
// 0 == Hardware compatibility off
// NOTE:
// on some hardware you can use "hardware compatibility off" and it will still run
#define HARDWARE_SC 0

#if HARDWARE_SC == 1
#	define ENABLE_FAT32 0
#	define ENABLE_ATA 0
#	define ENABLE_ULIME 0
#	define ENABLE_GLIME 0
#else
#	define ENABLE_FAT32 1
#	define ENABLE_ATA 1
#	define ENABLE_ULIME 1
#	define ENABLE_GLIME 1
#endif

#define X64   1
#define RISCV 0
#define ARM64 0

// formats
#define EMEX  "emex"
#define EMEX1 "EMEX"
#define EMEX2 "[emex]"
#define EMEX3 "[EMEX]"
#define EMEX4 "emx"
#define EMX   EMEX4


// 1 == enable automatic formatting
// 0 == require manual formatting
// NOTE: this *CAN* erase *DATA* on your disk if enabled!
#define OVERWRITEALL 0

//#define NULL_ 1