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

CC     := x86_64-elf-gcc
LD     := x86_64-elf-ld
LIBC   := ../../libc
LIBS_D := ../../libs

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 \
          -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include                                 \
          -I$(LIBS_D)/tinygl/include                        \
          -I$(LIBS_D)/tinygl/src                            \
          -I$(LIBS_D)/eXui                                  \
          -I$(LIBS_D)/libdesktop                            \
          -I$(LIBS_D)/libfont

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

LIBS := $(LIBS_D)/eXui/libexui.a \
        $(LIBS_D)/libdesktop/libdesktop.a \
        $(LIBS_D)/libfont/libfont.a \
        $(LIBS_D)/tinygl/libTinyGL.a \
        $(LIBC)/build/libc.a

all: gears.elf

gears.elf: gears.o $(LIBC)/build/crt0.o $(LIBS)
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o gears.o $(LIBS) -o $@

gears.o: gears.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o gears.elf

.PHONY: all clean