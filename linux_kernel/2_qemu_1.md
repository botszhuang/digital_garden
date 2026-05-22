---
title: "QEMU"
auther: "Botsz"
date: 2026-05-22
---
## Installation
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install qemu-system-arm qemu-system-x86 alpine-linux-install

sudo apt install -y ovmf
ls -l /usr/share/qemu/edk2-aarch64-code.fd

# or download Alpine ISO
wget https://dl-cdn.alpinelinux.org/alpine/v3.23/releases/aarch64/alpine-standard-3.23.4-aarch64.iso

# create a harddisk
qemu-img create -f qcow2 alpine.qcow2 16G

Formatting 'alpine.qcow2', fmt=qcow2 cluster_size=65536 extended_l2=off compression_type=zlib size=17179869184 lazy_refcounts=off refcount_bits=16

# Check the harddisk
qemu-img info alpine.qcow2
```
### ARM64
```bash
qemu-system-aarch64  -machine virt  -cpu cortex-a72  -m 2048  -smp 2  -drive file=alpine.qcow2,format=qcow2  -cdrom alpine-standard-3.23.4-aarch64.iso  -nographic  -enable-kvm
```
| Parameter | Meaning|
| :--- | :---|
|`-M virt`| Virtual Machine type
| `-cpu cortex-a15`| CPU type to emulate 
| `-m 512`| Allocate 512MB of RAM |
| `-drive`| Hard disk configuration |
| `-cdrom`| CD-ROM / ISO image |
| `-nographic`| No graphical interface (saves resources)|


## 🎯 KVM Permission Issue
```bash
# Check if CPU supports virtualization
grep -o 'vmx\|svm' /proc/cpuinfo

# Check if KVM module is loaded
lsmod | grep kvm

# Check if /dev/kvm exists
ls -l /dev/kvm

# All-in-one check
kvm-ok
# Output example:
# INFO: /dev/kvm exists
# INFO: KVM acceleration can be used
```
### Solution 1：remove -enable-kvm 
```bash
qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a72 \
  -m 2048 \
  -smp 4 \
  -drive file=alpine.qcow2,format=qcow2 \
  -cdrom alpine-standard-3.23.4-aarch64.iso \
  -nographic \
```
### Solution 2: Add User to KVM Group
If KVM environment is normal, but the issue is insufficient permissions
```bash
# Add User to kvm Group
# Add current user to kvm group
sudo usermod -aG kvm $USER

# Or add a specific user
sudo usermod -aG kvm username

# Activate group immediately
newgrp kvm

# Verify Permission Change
id
# groups=...kvm... 

```
## 🎯 ARM aarch64 emulator 啟動問題
### 1. 問題識別
初始錯誤：系統安裝的是 x86_64 的 OVMF 韌體，但需要啟動的是 ARM aarch64 虛擬機
```
Could not find ROM image
```
### 2. 診斷過程
| 步驟	| 執行命令 |	發現 |
| :--- | :--- | :--- |
| 1 |	dpkg -l \| grep ovmf	| 找到 ovmf 2024.02-2ubuntu0.8 (x86_64) |
| 2	| dpkg -L ovmf \| grep "\.fd$"	| 沒有 edk2-aarch64-code.fd 檔案 |
| 3	| 搜尋系統韌體檔案 |	僅找到 JSON 配置，無實際韌體 |
| 4	| 分析輸出	| **架構不匹配**：x86_64 vs aarch64 |

#### 完整檢查 OVMF 和 qemu-efi 安裝狀態
```bash
# 檢查 OVMF 套件
dpkg -l | grep ovmf

# 檢查 qemu-efi-aarch64 套件
dpkg -l | grep qemu-efi

# 列出 OVMF 套件包含的所有檔案（特別是 .fd 檔案）
dpkg -L ovmf 2>/dev/null | grep "\.fd$"

# 列出 qemu-efi-aarch64 套件包含的所有檔案
dpkg -L qemu-efi-aarch64 2>/dev/null | grep "\.fd$"
```

#### 搜尋所有 .fd 韌體映像檔
```bash
# 搜尋所有 .fd 檔案
sudo find /usr -name "*.fd" -type f 2>/dev/null

# 或者搜尋 qemu 目錄中的所有內容
ls -laR /usr/share/qemu/ 2>/dev/null | grep -E "\.fd|edk2"
```

### 3. 解決方案
#### 1. 移除錯誤的 x86 韌體
```bash
sudo apt remove -y ovmf
```
#### 2. 安裝正確的 ARM 韌體
```bash
sudo apt install -y qemu-efi-aarch64
```
#### 3.驗證韌體檔案
```bash
dpkg -L qemu-efi-aarch64 | grep "\.fd$"
```
找到的韌體：
```
/usr/share/qemu-efi-aarch64/QEMU_EFI.fd ✅
```
#### 4. Run the emulator
```bash
timeout 60 qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a72 \
  -m 2048 \
  -smp 2 \
  -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
  -drive file=alpine.qcow2,format=qcow2 \
  -cdrom alpine-standard-3.23.4-aarch64.iso \
  -nographic 2>&1 | tee qemu_output.log

cat qemu_output.log
```

#### 5. 成功啟動✅ emulator 成功運行：
```
Booting `Linux lts'
OpenRC 0.63 is starting up Linux 6.18.22-0-lts (aarch64)
 * /proc is already mounted
 * Mounting /run ... [ ok ]
```
#### Summary
|要素	|詳情|
|:--- |:--- |
| 問題	| 架構不匹配（x86_64 vs aarch64）|
| 根本原因 |	安裝了錯誤的韌體套件 |
| 解決時間 |	多步驟診斷 → 精確識別 → 快速修復 |
| 最終狀態 |	✅ 虛擬機成功啟動並執行 Alpine Linux |

## First RUN ARM aarch64 emulator
```bash
# run emulator
qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a72 \
  -m 2048 \
  -smp 2 \
  -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
  -drive file=alpine.qcow2,format=qcow2 \
  -cdrom alpine-standard-3.23.4-aarch64.iso \
  -net nic -net user \
  -nographic

# enter user name
localhost login: root

# Press Enter (empty password)
Password:    

# install Apline to the disk
setup-alpine

# When it asks for the disk mode, type **sys** (this stands for a traditional system install on disk)

# Installation is complete. Please reboot.
reboot
```

## RUN ARM aarch64 emulator
```bash
# run emulator
qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a72 \
  -m 2048 \
  -smp 2 \
  -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
  -drive file=alpine.qcow2,format=qcow2 \
  -net nic -net user \
  -nographic

# System will shut down
localhost:~# poweroff  
```