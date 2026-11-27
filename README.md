# ToDo

### BootROM (Stage 1) (truely read-only)
- [x] Asymetric code signing + validation without OpenSSL.
- [x] FAT driver for disks. (for early development)
- [x] img4 loader and verifier
- [x] load a cryptographically signed file called `/bootloader.img4`

### eBoot (Stage 2)
- [x] Bring a logo to life by loading a cryptographically signed file called `/logo.img4`
- [ ] Bring the kernel to life by loading a cryptographically signed file called `/kernel.img4`

### Kernel (Stage 3)
- [ ] Pointer Authentication Codes(PAC) setup
- [ ] Interrupt Controller Management (ICM)
- [ ] Physical Memory Management (PMM)
- [ ] Virtual Memory Management (VMM)
