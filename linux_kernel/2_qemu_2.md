---
title: "Qemu - apline linux"
auther: "Botsz"
date: 2026-05-22
---
## 1. Update
```bash
apk update

# output
# 3.23.4 [/media/vdb/apks]
# or
# v3.23.4-253-g3f34f48081a [http://dl-cdn.alpinelinux.org/alpine/v3.23/main]
# OK: 5887 distinct packages available

```
### Edit the repositories 
```bash
vi /etc/apk/repositories

# insert
# default official online repositories
# https://dl-cdn.alpinelinux.org/alpine/v3.23/main
# https://dl-cdn.alpinelinux.org/alpine/v3.23/community
```
## 2. Install the essential tool
```bash
apk add build-base gcc make vim

# ( 1/23) Installing libgcc (15.2.0-r2)
# ( 2/23) Installing jansson (2.14.1-r0)
# ( 3/23) Installing libstdc++ (15.2.0-r2)
# ...
```
What these packages do:

- `build-base` / `gcc` / `make`: The core C compiler toolchain required to compile anything.
- `vim` : editor

### Verify the Headers Setup
Unlike Ubuntu or Debian, Alpine Linux's linux-lts-dev package installs the kernel source tree directly into `/usr/src/linux-headers-6.18.32-0-lts/`. However, it does not automatically create the **standard symbolic link** (`/lib/modules/.../build`) that external module Makefiles typically look for.

You just need to create that symlink manually, and you'll be ready to compile.

```bash
# Run this command to bridge the gap:
ln -s /usr/src/linux-headers-$(uname -r) /lib/modules/$(uname -r)/build

#Let's verify it
ls -l /lib/modules/$(uname -r)/build

# output:
# lrwxrwxrwx    1 root     root            36 May 22 11:49 /lib/modules/6.18.32-0-lts/build -> /usr/src/linux-headers-6.18.32-0-lts

```

## 3. Mount the Host's Folder
### step1: Modify your Host's QEMU Launch Script (The Host)
```bash
qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a72 \
  -m 2048 \
  -smp 2 \
  -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
  -drive file=alpine.qcow2,format=qcow2,if=virtio \
  -net nic -net user \
  -nographic \
  -fsdev local,path=/home/user/kernel-dev,id=hostshare,security_model=none \
  -device virtio-9p-pci,fsdev=hostshare,mount_tag=shared_dir
```
- `path=/home/user/kernel-dev`: The absolute path to the directory on your actual host computer.

- `mount_tag=shared_dir`: A unique nickname or "tag" you give this share. Your Alpine guest will look for this exact name.

### step 2: Mount it inside Alpine (The Guest)
```bash
# 1. Create a folder where you want the files to appear
mkdir -p /mnt/host

# 2. Mount using the mount_tag defined in your QEMU script
mount -t 9p -o trans=virtio shared_dir /mnt/host
```
Type `ls /mnt/host` and the host folder is available inside the Alpine terminal!
### Step 3: Make it Persistent (Optional)
```bash 
vi /etc/fstab

# And add this line to the bottom of the file:
# shared_dir  /mnt/host  9p  trans=virtio,version=9p2000.L,_netdev  0  0
```
Now, Alpine will automatically mount your host directory into /mnt/host on every boot.