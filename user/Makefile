#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (c) 2026 emex-foundation
#
# FILE: Makefile
# CREATED BY: emex
# MODIFIED BY: --
#
#

all:
	@$(MAKE) -C libc
	@$(MAKE) -C libs/emxfb0
	@$(MAKE) -C libs/libfont
	@$(MAKE) -C libs/libdesktop
	@$(MAKE) -C libs/eXui
	@$(MAKE) -C libs/libvad
	@$(MAKE) -C libs/tinygl
	@$(MAKE) -C libs/libbmp

	@$(MAKE) -C apps/system
	@$(MAKE) -C apps/shell
	@$(MAKE) -C apps/terminal
	@$(MAKE) -C apps/welcome
	@$(MAKE) -C apps/login
	@$(MAKE) -C apps/desktop
	@$(MAKE) -C apps/sysinfo
	@$(MAKE) -C apps/filemanager
	@$(MAKE) -C apps/template
	@$(MAKE) -C apps/gears

	@$(MAKE) -C bin/echo
	@$(MAKE) -C bin/hello
	@$(MAKE) -C bin/ls
	@$(MAKE) -C bin/tree
	@$(MAKE) -C bin/cd
	@$(MAKE) -C bin/cat
	@$(MAKE) -C bin/lsblk
	@$(MAKE) -C bin/reboot
	@$(MAKE) -C bin/shutdown
	@$(MAKE) -C bin/rm
	@$(MAKE) -C bin/touch
	@$(MAKE) -C bin/uptime
	@$(MAKE) -C bin/uname
	@$(MAKE) -C bin/ps
	@$(MAKE) -C bin/pwd
	@$(MAKE) -C bin/meminfo
	@$(MAKE) -C bin/dofetch
	@$(MAKE) -C bin/wav

clean:
	@$(MAKE) -C libc clean
	@$(MAKE) -C libs/emxfb0 clean
	@$(MAKE) -C libs/libfont clean
	@$(MAKE) -C libs/libdesktop clean
	@$(MAKE) -C libs/eXui clean
	@$(MAKE) -C libs/libvad clean
	@$(MAKE) -C libs/tinygl clean
	@$(MAKE) -C libs/libbmp clean

	@$(MAKE) -C apps/system clean
	@$(MAKE) -C apps/shell clean
	@$(MAKE) -C apps/terminal clean
	@$(MAKE) -C apps/welcome clean
	@$(MAKE) -C apps/login clean
	@$(MAKE) -C apps/desktop clean
	@$(MAKE) -C apps/sysinfo clean
	@$(MAKE) -C apps/filemanager clean
	@$(MAKE) -C apps/template clean
	@$(MAKE) -C apps/gears clean

	@$(MAKE) -C bin/echo clean
	@$(MAKE) -C bin/hello clean
	@$(MAKE) -C bin/ls clean
	@$(MAKE) -C bin/tree clean
	@$(MAKE) -C bin/cd clean
	@$(MAKE) -C bin/cat clean
	@$(MAKE) -C bin/lsblk clean
	@$(MAKE) -C bin/reboot clean
	@$(MAKE) -C bin/shutdown clean
	@$(MAKE) -C bin/rm clean
	@$(MAKE) -C bin/touch clean
	@$(MAKE) -C bin/uptime clean
	@$(MAKE) -C bin/uname clean
	@$(MAKE) -C bin/ps clean
	@$(MAKE) -C bin/pwd clean
	@$(MAKE) -C bin/meminfo clean
	@$(MAKE) -C bin/dofetch clean
	@$(MAKE) -C bin/wav clean

.PHONY: all clean