CC   := x86_64-elf-gcc
AR   := x86_64-elf-ar
LIBC := ../../libc

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64     \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include

all: libfont.a

libfont.a: libfont.o
	$(AR) rcs $@ $^

libfont.o: libfont.c libfont.h data/font8x12.h data/font8x12_bold.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o libfont.a

.PHONY: all clean