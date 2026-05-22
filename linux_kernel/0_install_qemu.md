---
title: "Install QEMU and KVM"
auther: "Botsz"
date: 2026-05-20
---
# Installation
## Install QEMU and KVM
```bash
sudo apt install qemu-kvm qemu-system libvirt-daemon-system libvirt-clients bridge-utils
```
- QEMU: Quick Emulator
- KVM: Kernel-based Virtual Machine

## Download a tiny test image
Create a new folder, move into it, and download the standard Alpine Linux virtual ISO:
```bash
mkdir qemu-test && cd qemu-test
curl -O https://dl-cdn.alpinelinux.org/alpine/v3.19/releases/x86_64/alpine-virt-3.19.1-x86_64.iso
```
Run the following command to boot the ISO directly in a basic virtual machine:
```bash
qemu-system-x86_64 -cdrom alpine-virt-3.19.1-x86_64.iso -m 512
``` 
# Kernel Development
## Setting Up the Host for Kernel Development
```bash
# Update package indices
sudo apt update

# Install build tools and matching kernel headers
sudo apt install build-essential kmod linux-headers-$(uname -r)
```
- ```kmod``` is short for Kernel Modules.

In the Linux world, kmod refers to two closely related things: the general concept of **Loadable Kernel Modules** (LKMs), and the actual **user-space toolset** for management.

## 1. Loadable Kernel Modules (LKMs)
Linux kernel is highly modular. 
Instead of recompiling the entire operating system for any new deivce, the kernel can load the mini-programs/modules on the fly. These are Kernel Modules (```.ko``` files).

![kernel Architecture](./pic/Kernel_Architecture.png)
>Image Source : https://github.com/Aditya-1208/linux_kernel_modules

## 2. The `kmod` Package
The standard suite of command-line utilities used to control these modules from user space.
| Command | What it does | Example |
| :--- | :--- | :--- |
| **`lsmod`** | **L**ist **Mod**ules: Shows every driver currently loaded into your running kernel. | `lsmod \| grep vbox` |
| **`insmod`** | **In**sert **Mod**ule: Directly injects a raw `.ko` file into kernel memory space. | `sudo insmod my_driver.ko` |
| **`rmmod`** |	**R**e**m**ove **Mod**ule: Unloads and unlinks a running driver from kernel memory.| `sudo rmmod my_driver` |
| **`modprobe`** |	Smart loader: Unlike `insmod`, it automatically looks up and loads a module's prerequisites/dependencies.	| **`sudo modprobe vboxdrv`** |
| **`modinfo`** | **Mod**ule **Info**rmation: Reads a `.ko` file's metadata (Author, License, Version description).	| `modinfo my_driver.ko`|