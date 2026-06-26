<div align="center">
	<h1> emexOS </h1>


a simple x86_64 Operating System in C made from scratch

![GitHub License](https://img.shields.io/github/license/emexos/emexOS1?style=for-the-badge&label=License&labelColor=black&color=white)
![GitHub repo size](https://img.shields.io/github/repo-size/emexos/emexOS1?style=for-the-badge&label=size&labelColor=black&color=white)
![GitHub Repo stars](https://img.shields.io/github/stars/emexos/emexos1?style=for-the-badge&labelColor=black&color=white)
	<div>
		<img src="include/screenshots/file_manager.png" width="23%" />
		<img src="include/screenshots/bootup.png" width="23%" />
		<img src="include/screenshots/login.png" width="23%" />
		<img src="include/screenshots/terminal.png" width="23%" />
	</div>
	-> emexOS is not based on any existing kernel <-
	<br>
	<a href="https://discord.gg/Cbeg3gJzC7">
		<img src="https://img.shields.io/badge/Join%20the%20Discord-Black?style=for-the-badge&logo=discord&logoColor=white&color=black" alt="Discord Badge" />
	</a>
</div>

NOTE:
EMEXOS DEVELOPEMENT WILL TAKE A PAUSE AFTER 1st August 2026

## What is emexOS?
[emexOS](https://github.com/emexlab/emexOS) is a small graphical operating system entirely written from scratch in C by emex and [all its contributors](https://github.com/emexlab/emexOS/graphs/contributors). It's designed for customizer and retro-enthusiasts, who want to fully change the look of their system and love retro-style.

## Build Dependencies 
For building and compiling emexOS, ensure you have the following installed:
- x86_64 GCC cross-compiler - This is our preferred compiler for the source code. Currently, emexOS only supports 64-bit x86 machines, but we plan to support more architectures in the future.
- [NASM](https://www.nasm.us/) - Assembler.
- [QEMU](https://www.qemu.org/) - Our preferred emulator.
- [Xorriso](https://www.gnu.org/software/xorriso/) - ISO creation.
- [Git](https://git-scm.com/) and [wget](https://www.gnu.org/software/wget/) - To fetch dependencies.
- [CPIO](https://www.gnu.org/software/cpio/) (use brew install cpio) - For the RAM disk.

You can also use [Zig](https://ziglang.org/) instead of GCC if you prefer to.

## For Nix Users 
Ensure you have flakes enabled first, then run `nix develop` to enter the provided development shell which will have all build dependencies and such installed for you. I'd recommend updating the flake using `nix flake update` as it's sometimes not updated for long periods of time.

## Building and Compiling 
With all build dependencies installed, you can finally build and run emexOS.

**Using gnu-make**
- `make fetchDeps` - Fetch all libraries and such that emexOS depends on e.g. Limine. `git` and `wget` are used for this.
- `make` - Build emexOS.
- `make run` - Emulate emexOS using QEMU.
- `make clean` - Clean up all build outputs.

**Using Zig**
- `zig build` - Fetch, build and run.
- `zig build -Dnofetch` - Build and run.
- `zig build -Dclean` - Remove (cache, output), fetch, build and run.
- `zig build -Dnofetch -Dclean` - Remove (cache, output), build and run.

## More Screenshots
<div align="center">
	<div>
		<img src="include/screenshots/terminal_sysinfo.png" width="23%" />
		<img src="include/screenshots/bootloader.png" width="23%" />
		<img src="include/screenshots/file_manager2.png" width="23%" />
		<img src="include/screenshots/more_windows.png" width="23%" />
	</div>
</div>

## Desktop
After booting, you will get into login and will be asked for a password and username, both are `emex` by default. but you can change it in `user/apps/login/login.c` .

Currently, no SMP support is available just yet so emexOS runs using only a single core.

<div align="right">
	<p>readme by emex, Voxi0 and Bi Moz </p>
</div>
