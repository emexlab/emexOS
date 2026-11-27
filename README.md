# ToDo

### eBoot (BootROM)
- [ ] Asymetric code signing + validation without OpenSSL.
- [ ] FAT driver for disks.
- [ ] load some kind of cryptographically signed "bootcache" from `/boot.bc` containing the list of SHA384 hashes allowed to load next to a list of payloads to load into memory and which and which hash is for which payload.
- [ ] payloads it loads shall be named like `/kernel.bpld` or `/sptm.bpld` for example.
