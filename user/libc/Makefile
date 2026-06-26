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

CC  := x86_64-elf-gcc
AR  := x86_64-elf-ar
INC := include
BLD := build

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-red-zone -Wall -Wextra \
          -std=gnu11 -I$(INC) -Isrc

SRCS := src/errno.c src/string.c src/stdlib.c src/unistd.c src/fcntl.c src/stdio.c src/dirent.c src/ioctl.c src/system.c src/wait.c src/sinfo.c src/math.c src/vt_proc.c
OBJS := $(patsubst src/%.c, $(BLD)/%.o, $(SRCS))

all: $(BLD)/libc.a $(BLD)/crt0.o

$(BLD)/libc.a: $(OBJS)
	$(AR) rcs $@ $^

$(BLD)/%.o: src/%.c | $(BLD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BLD)/crt0.o: src/crt0.S | $(BLD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BLD):
	mkdir -p "$@"

clean:
	rm -rf "$(BLD)"

.PHONY: all clean