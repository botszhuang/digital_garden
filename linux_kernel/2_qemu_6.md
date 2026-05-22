---
title: "Initramfs - Busybox"
auther: "Botsz"
date: 2026-05-23
---
An `initramfs.cpio.gz` file is a compressed archive containing a minimal root filesystem that the Linux kernel loads into RAM during the boot process, before the actual root filesystem is mounted
### 1. Prepare a Working Directory: 
Create a dedicated directory for your initramfs content.
```bash
mkdir initramfs_root
cd initramfs_root
```
### 2. Create Essential Directories: 
Set up the basic directory structure that a minimal Linux system expects.
```bash
mkdir -p bin dev etc lib proc sbin sys usr
mkdir -p usr/bin usr/sbin
```
### 3. Create the `/init` Script: 
This script is the first program executed by the kernel. It's crucial for setting up the environment and eventually mounting the real root filesystem.
```bash
vim init
```
```
#!/bin/sh
mount -t proc none /proc' 
mount -t sysfs none /sys' 
mount -t devtmpfs none /dev'  # Or create device nodes manually
exec /bin/init # Or switch_root to the real root
```
```bash
chmod +x init
```

**Note**: A real `/init` script would be much more complex, including logic for detecting and mounting the actual root filesystem, handling LVM, encryption, etc. For a simple test, `exec /bin/sh` could also be used to get a shell.

### 4. Add Binaries and Libraries with BusyBox

#### 4.1 Use a Pre-compiled BusyBox
```bash
# Download a static BusyBox binary (x86_64 example)
wget https://busybox.net/downloads/binaries/1.35.0-x86_64-linux-musl/busybox

# Make it executable
chmod +x busybox

# Move it to bin/
mv busybox bin/busybox
```

#### 4.2: Create Symlinks
Once BusyBox is in place, create symlinks for all the commands it provides:

```bash
cd bin

# Create symlinks for essential commands
ln -s busybox sh
ln -s busybox ls
ln -s busybox cat
ln -s busybox echo
ln -s busybox mkdir
ln -s busybox mount
ln -s busybox umount

```
or
```bash
cd bin
./busybox --list | while read cmd; do ln -s busybox $cmd; done
```
Verify the symlinks:
```bash
cd ..
ls -la bin/ | head -20
```
#### 4.3. Copy Required Libraries
Find what libraries BusyBox needs:
```bash
ldd bin/busybox
```
Output:
```
   not a dynamic executable
```
To compile BusyBox statically, it might not need extra libraries.

#### 4.4. Update the `/init` Script
Modify the `/init` script to use BusyBox:
```bash
vim init
```
Plain text:
```
#!/bin/sh

# Mount essential filesystems
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev

# Print a welcome message
echo "Welcome to initramfs"

# Drop to a shell for testing/debugging
exec /bin/sh
```
```bash
chmod +x init
```
#### Verify symlink for init:
```bash
ln -s /bin/busybox init

chmod +x init
ls -la init
```
### Verification Checklist
```bash
cd initramfs_root

# Check BusyBox is present
ls -lh bin/busybox

# Check symlinks exist
ls bin/sh bin/ls bin/cat

# Check libraries are copied (if needed)
ls lib/x86_64-linux-gnu/libc.so.6 2>/dev/null || echo "Static binary - no libc needed"

# Verify init script is executable
ls -la init
file init
cat init
```
### 5. Create Device Nodes
```bash
sudo mknod dev/console c 5 1
sudo mknod dev/null c 1 3
sudo mknod dev/tty c 5 0
sudo mknod dev/tty1 c 4 1
sudo mknod dev/sda b 8 0
sudo mknod dev/sda1 b 8 1

# Verify
ls -la dev/
```

### 6. Package the Initramfs
```bash
# Go back to initramfs root

# Create the compressed cpio archive
find . | cpio -o -H newc | gzip > ../initramfs.cpio.gz

# Verify the output
ls -lh initramfs.cpio.gz
file initramfs.cpio.gz
gunzip -c initramfs.cpio.gz | cpio -t | head -20
```
## Final Product
```
initramfs.cpio.gz
```
## ✅ Ready to boot


