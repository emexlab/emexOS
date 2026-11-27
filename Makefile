# emexOS Makefile lol

PROJECT_ROOT := $(CURDIR)
EMEX64_LLVM := $(PROJECT_ROOT)/EmexToolchain/llvm/install/bin
FIRMWARE := $(PROJECT_ROOT)/firmware/BootROM/firmware.img

all: $(FIRMWARE)

$(EMEX64_LLVM)/clang:
	EmexToolchain/llvm/build.sh

$(FIRMWARE): $(EMEX64_LLVM)/clang
	$(MAKE) -C firmware/BootROM build

clean:
	$(MAKE) -C firmware/BootROM clean
