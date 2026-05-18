#!/bin/bash
set -e

# Configuration
TINYGL_REPO="https://github.com/C-Chads/TinyGL.git"
DEST_DIR="src/userspace/libs/tinygl"

echo ":: Porting TinyGL to emexOS..."

# 1. Clone TinyGL
if [ ! -d "$DEST_DIR" ]; then
    echo ":: Cloning TinyGL into $DEST_DIR..."
    git clone "$TINYGL_REPO" "$DEST_DIR"
else
    echo ":: TinyGL already exists in $DEST_DIR, skipping clone."
fi

cd "$DEST_DIR"

# 2. Generate Makefile for emexOS
echo ":: Generating Makefile..."
cat << 'EOF' > Makefile
CC   := x86_64-elf-gcc
AR   := x86_64-elf-ar
LIBC := ../../libc

# NOTE: SSE is required for 64-bit float handling and is enabled in the kernel now.
CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 \
          -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include -Iinclude -Isrc -D__EMEX__

# TinyGL sources (adjusted for C-Chads fork)
SRCS := src/api.c src/arrays.c src/clear.c src/clip.c src/get.c \
        src/init.c src/light.c src/matrix.c src/memory.c src/misc.c \
        src/msghandling.c src/select.c src/specbuf.c src/texture.c \
        src/vertex.c src/zbuffer.c src/zline.c src/zmath.c \
        src/ztriangle.c src/zraster.c src/ztext.c src/list.c \
        src/zpostprocess.c src/image_util.c

OBJS := $(SRCS:.c=.o)

all: libTinyGL.a

libTinyGL.a: $(OBJS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) libTinyGL.a

.PHONY: all clean
EOF

# 3. Patch TinyGL to use system <math.h> headers
echo ":: Patching TinyGL..."
grep -l "#include <math.h>" src/*.c src/*.h | xargs sed -i 's/"math.h"/<math.h>/' || true

echo ":: TinyGL porting setup complete."
echo ":: To build libc and TinyGL, run: "
echo "   make -C src/userspace/libc clean && make -C src/userspace/libc"
echo "   make -C src/userspace/libs/tinygl"
