---
title: "Create the Initramfs - Busybox"
auther: "Botsz"
date: 2026-05-23
---

# ⚠️ This page is kept for record purposes only. 
# The content is deprecated and no longer recommended for use. 
# Please refer to [Initramfs - Busybox](2_qemu_6.md) instead.

# 1. Download & Build BusyBox
```bash
wget https://busybox.net/downloads/busybox-1.38.0.tar.bz2
tar xjf busybox-1.38.0.tar.bz2
cd busybox-1.38.0
```
## Quickest Fix: Disable tc
BusyBox relies on Linux kernel headers for traffic control (`tc`). Since Linux Kernel 6.8, the kernel developers officially dropped CBQ (Class-Based Queueing) code, the corresponding applets in `networking/tc.c` caused compilation errors.
```bash
make menuconfig
```
Navigate to:
```
Networking Utilities
  └─ [ ] tc (Traffic Control)
```
**Uncheck the box** and exit (save the config).

Then rebuild:
```bash
make clean
make -j$(nproc)
make install

```
# 2. Create Initramfs Directory Structure
```bash
mkdir -p initramfs/{bin,sbin,etc,proc,sys,dev,root}

# Copy BusyBox binary
cp _install/bin/busybox initramfs/bin/
```
### Create symlinks for BusyBox applets
```bash
# Navigate to initramfs/bin
cd initramfs/bin

# List all available applets and create symlinks
for applet in $(./busybox --list); do
  ln -s busybox $applet
done

# Go back to previous directory
cd -
```
### Verify It Worked
```bash
# Check how many symlinks were created
ls -la initramfs/bin | wc -l

# See some examples
ls -l initramfs/bin | head -20
```
### Package everything into the initramfs
```bash
cd initramfs

# Create CPIO archive
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz

# Verify
ls -lh ../initramfs.cpio.gz

# output
# -rw-rw-r-- 1 botsz botsz 9.9M May 23 10:27 ../initramfs.cpio.gz
```

## ✅ Ready to boot