/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: uptime.c
 * CREATED BY: xTrayambak
 * MODIFIED BY: emex
 *
 */

#include <unistd.h>
#include <stdio.h>

#include <emx/sinfo.h>

static void show_uptime(unsigned long uptime_secs)
{
	unsigned long hours = uptime_secs / 3600;
	unsigned long mins = (uptime_secs % 3600) / 60;
	unsigned long secs = uptime_secs % 60;

	printf("up  %lu:%02lu:%02lu\n", hours, mins, secs);
}

int main(void)
{
	struct sysinfo_t info;
	int ret = sysinfo(&info);

	if (ret < 0)
	{
		perror("sysinfo");
		return 1;
	}

	show_uptime(info.uptime);
    return 0;
}