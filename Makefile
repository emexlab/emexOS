# emexOS Makefile lol

PROJECT_ROOT := $(CURDIR)
EMEX64_LLVM := $(PROJECT_ROOT)/EmexToolchain/llvm/install/bin
FIRMWARE := $(PROJECT_ROOT)/firmware/BootROM/firmware.img
NVRAM := $(FIRMWARE).nvram

IMG4_KEYS ?= $(HOME)/.emexlabs/img4
BOOTROM_C := firmware/BootROM/c
IMG4_SOURCES := tools/img4/main.c $(BOOTROM_C)/img4.c $(BOOTROM_C)/x509.c $(BOOTROM_C)/der.c $(BOOTROM_C)/monocypher.c $(BOOTROM_C)/monocypher-ed25519.c

LOGO_PNG := $(wildcard eBoot/logo.png)
LOGO_SIZE ?= 128

CC := $(EMEX64_LLVM)/clang
LD := $(EMEX64_LLVM)/ld.lld
OBJCOPY := $(EMEX64_LLVM)/llvm-objcopy

EBOOT_SOURCES := eBoot/main.c eBoot/disk.c eBoot/string.c $(BOOTROM_C)/iso_font.c $(BOOTROM_C)/fat.c $(BOOTROM_C)/img4.c $(BOOTROM_C)/x509.c $(BOOTROM_C)/der.c $(BOOTROM_C)/monocypher.c $(BOOTROM_C)/monocypher-ed25519.c
EBOOT_OBJECTS := $(addprefix Build/eboot/,$(notdir $(EBOOT_SOURCES:.c=.o)))

CFLAGS := --target=emex64-unknown-elf -ffreestanding -fno-builtin -fno-stack-protector -fpie -O2 -Wall \
          -ffunction-sections -fdata-sections -fno-jump-tables -include eBoot/hidden.h \
          -IeBoot -IBuild/include -I$(BOOTROM_C)/include -IEmexToolchain/EmexToolchain/include/public
LDFLAGS := -pie --no-dynamic-linker --gc-sections -T eBoot/eboot.lds

DISK_FILES := Build/bootloader.img4 $(if $(LOGO_PNG),Build/logo.img4) Build/kernelcache.img4

EMEX64VM := EmexToolchain/build/bin/emex64vm

KERNELCACHE_SOURCES := kernel/main.c
KERNELCACHE_OBJECTS := $(addprefix Build/kernelcache/,$(notdir $(KERNELCACHE_SOURCES:.c=.o)))

all: $(EMEX64VM) firmware Build/disk.img

Build Build/eboot Build/include Build/kernelcache:
	mkdir -p $@

Build/img4: $(IMG4_SOURCES) | Build
	cc -O2 -Wall -I$(BOOTROM_C)/include -o $@ $(IMG4_SOURCES)

Build/logo: tools/logo/main.c eBoot/logo.h | Build
	cc -O2 -Wall -IeBoot -o $@ tools/logo/main.c -lz

Build/nvram: tools/nvram/main.c $(BOOTROM_C)/nvram.c | Build
	cc -O2 -Wall -I$(BOOTROM_C)/include -o $@ tools/nvram/main.c $(BOOTROM_C)/nvram.c

$(IMG4_KEYS)/signing.key: | Build/img4
	mkdir -p "$(dir $(IMG4_KEYS))"
	Build/img4 init "$(IMG4_KEYS)"

Build/include/img4_root.h: Build/img4 $(IMG4_KEYS)/signing.key | Build/include
	Build/img4 rom "$(IMG4_KEYS)" $@

Build/eboot/%.o: eBoot/%.c eBoot/hidden.h Build/include/img4_root.h | Build/eboot $(EMEX64_LLVM)/clang
	$(CC) $(CFLAGS) -c $< -o $@

Build/eboot/%.o: $(BOOTROM_C)/%.c eBoot/hidden.h | Build/eboot $(EMEX64_LLVM)/clang
	$(CC) $(CFLAGS) -c $< -o $@

Build/bootloader.bin: $(EBOOT_OBJECTS) eBoot/eboot.lds
	$(LD) $(LDFLAGS) -o Build/bootloader.elf $(EBOOT_OBJECTS)
	$(OBJCOPY) -O binary Build/bootloader.elf $@

Build/bootloader.img4: Build/bootloader.bin Build/img4 $(IMG4_KEYS)/signing.key Build/ecid
	Build/img4 --ecid $$(cat Build/ecid) --kbag prod:00112233445566778899aabbccddeeff:0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef sign "$(IMG4_KEYS)" ebot "eBoot 1.2.3" $< $@

Build/logo.bin: $(LOGO_PNG) Build/logo
	Build/logo $(LOGO_PNG) $@ $(LOGO_SIZE)

Build/logo.img4: Build/logo.bin Build/img4 $(IMG4_KEYS)/signing.key Build/ecid
	Build/img4 --ecid $$(cat Build/ecid) --kbag prod:00112233445566778899aabbccddeeff:0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef sign "$(IMG4_KEYS)" logo "logo" $< $@

Build/kernelcache/%.o: kernel/%.c eBoot/hidden.h Build/include/img4_root.h | Build/kernelcache $(EMEX64_LLVM)/clang
	$(CC) $(CFLAGS) -c $< -o $@

Build/kernelcache/%.o: $(BOOTROM_C)/%.c eBoot/hidden.h | Build/kernelcache $(EMEX64_LLVM)/clang
	$(CC) $(CFLAGS) -c $< -o $@

Build/kernelcache.bin: $(KERNELCACHE_OBJECTS) kernel/kernelcache.lds
	$(LD) $(LDFLAGS) -o Build/kernelcache.elf $(KERNELCACHE_OBJECTS)
	$(OBJCOPY) -O binary Build/kernelcache.elf $@

Build/kernelcache.img4: Build/kernelcache.bin Build/img4 $(IMG4_KEYS)/signing.key Build/ecid
	Build/img4 --ecid $$(cat Build/ecid) --kbag prod:00112233445566778899aabbccddeeff:0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef sign "$(IMG4_KEYS)" krnl "SNU 1.0.0" $< $@

Build/disk.img: $(DISK_FILES)
	rm -f $@ Build/esp.img
	dd if=/dev/zero of=Build/esp.img bs=1048576 count=64 2>/dev/null
	mformat -i Build/esp.img -v BOOTROM ::
	mcopy -i Build/esp.img $(DISK_FILES) ::/
	dd if=/dev/zero of=$@ bs=1048576 count=66 2>/dev/null
	sgdisk -n 1:2048:+64M -t 1:EF00 -c 1:"EFI system partition" $@ >/dev/null
	dd if=Build/esp.img of=$@ bs=1048576 seek=1 conv=notrunc 2>/dev/null
	rm -f Build/esp.img

Build/ecid: | Build
	printf '0x%016x\n' "$$(( 0x$$(od -An -N8 -tx1 /dev/urandom | tr -d ' \n') & 0x7fffffffffffffff ))" > $@

$(EMEX64_LLVM)/clang:
	EmexToolchain/llvm/build.sh

$(EMEX64VM):
	$(MAKE) -C EmexToolchain all

firmware: $(EMEX64_LLVM)/clang | $(IMG4_KEYS)/signing.key
	$(MAKE) -C firmware/BootROM build IMG4_KEYS="$(IMG4_KEYS)"

verbose: Build/nvram
	Build/nvram -f "$(NVRAM)" boot-args=-v

quiet: Build/nvram
	Build/nvram -f "$(NVRAM)" -d boot-args

clean:
	$(MAKE) -C firmware/BootROM clean
	$(MAKE) -C EmexToolchain clean
	rm -rf Build

clean-all: clean
	rm -rf EmexToolchain/llvm/build
	rm -rf EmexToolchain/llvm/install
	rm -rf EmexToolchain/llvm/llvm-project

run: all Build/nvram Build/ecid
	Build/nvram -f firmware/BootROM/firmware.img.nvram boot-args=
	$(EMEX64VM) -f $(FIRMWARE) --ecid $$(cat Build/ecid) --sata Build/disk.img --display on --display:resolution 800 600

run-verbose: all Build/nvram Build/ecid
	Build/nvram -f firmware/BootROM/firmware.img.nvram boot-args=-v
	$(EMEX64VM) -f $(FIRMWARE) --ecid $$(cat Build/ecid) --sata Build/disk.img --display on --display:resolution 800 600

.PHONY: all firmware verbose quiet clean run
