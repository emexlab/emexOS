# emexOS Makefile lol

all: EmexToolchain/llvm/install EmexToolchain/firmware/TianoCore/firmware.img

# dependencies
EmexToolchain/llvm/install:
	EmexToolchain/llvm/build.sh

EmexToolchain/firmware/TianoCore/firmware.img:
	EmexToolchain/firmware/TianoCore/build.sh
