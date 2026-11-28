#!/usr/bin/env bash
set -euo pipefail

usage="Usage: $0 <png_file> <size>"

png_file="${1:?$usage}"
size="${2:?$usage}"

Build/logo $png_file Build/logo.bin $size
Build/img4 --ecid $(cat Build/ecid) --kbag prod:00112233445566778899aabbccddeeff:0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef sign "$HOME/.emexlabs/img4" logo "logo" Build/logo.bin Build/logo.img4

rm -rf Build/disk.img
make Build/disk.img
