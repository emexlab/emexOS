/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: desktop.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#if (__EMEX_VERSION_MAJOR < 1)
    #define ICONSPATH "/emr/assets" // folder icons, file icons, desktop icons, app icons
#elif (__EMEX_VERSION_MAJOR == 1 && __EMEX_VERSION_MINOR == 0)
    #define ICONSPATH "/emr/system/desktop/icons"// folder icons, file icons, desktop icons, app icons
#else
	#define ASSETSPATH "/emr/assets" // folder icons, file icons, etc
    #define ICONSPATH  "/emr/system/desktop/icons" // desktop icons, app icons
#endif