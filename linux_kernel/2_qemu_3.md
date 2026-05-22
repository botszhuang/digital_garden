---
title: "QEMU - Building a Linux Kernel and Initramfs"
auther: "Botsz"
date: 2026-05-23
---
## Contents
- [Build the Kernel](#build-the-kernel)
- [Creat the Initramfs](#create-the-initramfs)
- [Run QEMU with Kernel and Initramfs](#running-qemu-with-kernel-and-initramfs)


## Build the Kernel
### Target: Kernel: arch/x86/boot/bzImage

This performs complete process to configure and build the Linux kernel using the default **x86_64 configuration**.

### Step 1: Prepare the Environment
```bash
sudo apt-get install build-essential libncurses-dev bison flex libssl-dev libelf-dev
```

### Step 2: Get the Kernel Source
```bash
# Download the kernel 
wget https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.19.tar.xz

tar -xf linux-6.19.tar.xz

cd linux-6.19
```

### Step 3: Configure the Kernel :star2:
```bash
# Clean previous buildmake clean
make clean

# Use the default x86_64 configuration
make x86_64_defconfig
```
This loads the default configuration and create a `.config` file with pre-optimized settings.

### Step 4: Build the Kernel :star2:
```bash
make -j$(nproc)

# or rebuild (watch for errors)
make -j$(nproc) 2>&1 | tee build.log
```
Compile the kernel (use -j for parallel jobs)

It takes several minutes depending on the hardware.

#### [ Notes ]
- `x86_64_defconfig` = Default configuration for 64-bit x86 systems
- `-j$(nproc)` = Parallel compilation using all available CPU cores

[back to contents](#contents)

## Create the Initramfs
### Target: Initramfs: initramfs.cpio.gz

### Solution 1: dracut
```bash
# Using dracut
dracut -o "network" -o "lvm" /path/to/initramfs.cpio.gz

# Or using mkinitramfs
mkinitramfs -o /path/to/initramfs.cpio.gz

```
### Solution 2: Busybox :star2:
[See Initramfs - Busybox](2_qemu_6.md)

## Running QEMU with Kernel and Initramfs
- ✅ Compiled kernel (bzImage)
- ✅ Initramfs (initramfs.cpio.gz)
```bash
qemu-system-x86_64 \
  -kernel ./linux-6.19/arch/x86/boot/bzImage \
  -initrd ./initramfs.cpio.gz \
  -append "console=ttyS0" \
  -m 512 \
  -nographic
```
output:
```
...
[    2.928220] Run /init as init process
[    2.986374] input: ImExPS/2 Generic Explorer Mouse as /devices/platform/i8042/serio1/input/input3
[    3.008273] mount (54) used greatest stack depth: 13928 bytes left
Welcome to initramfs
/bin/sh: can't access tty; job control turned off
/ # 
```
| Parameter	| Purpose |
| :--- | :--- |
|`-kernel` |	Path to the compiled kernel image (e.g., `bzImage`, `vmlinuz`) |
| `-initrd` |	Path to the initramfs file (e.g., `.cpio.gz, .cpio`) |
| `-append` |	Kernel command-line arguments |
| `-m` | Memory allocation (in MB) |
| `-nographic` | Run without GUI (text-only) |
| `-serial stdio` | Redirect serial output to console |

[back to contents](#contents)