---
title: "Run the Hello driver with QEMU"
auther: "Botsz"
date: 2026-05-27
---
## 1. Place your driver source code
Copy the `hello.c` file into the kernel source tree under the `drivers/misc/` directory:
```bash
cp hello.c linux-6.19/drivers/misc/hello.c
```
```c
# hello.c

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init hello_init(void)
{
    printk(KERN_ALERT "Hello, World!\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, World!\n");
}

module_init(hello_init);
module_exit(hello_exit);
```
### 1.1. The Header Files
- `<linux/init.h>`: Contains the macros `__init` and `__exit`, which optimize memory usage, as well as the functions `module_init()` and `module_exit()`.
- `<linux/module.h>`: Loaded by every loadable kernel module. It contains definitions for module **loading**, **unloading**, and **metadata**.
- `MODULE_LICENSE("GPL")`: the license for the module

### 1.2. The Initialization Function
- `static`: Restricts the visibility of this function to this file only. This prevents name collisions inside the massive kernel namespace.
- `__init`: A hint to the kernel that this function is only used during initialization. 
- `printk()`: The kernel equivalent of printf().
- `KERN_ALERT`: A log level priority. It ensures the message gets logged, and depending on your system configuration. [REF](./1_hello.md)

### 1.3 The Cleanup Function
- `__exit`: Similar to `__init`, this tells the kernel that this function is only used when unloading the module. 

### 1.4. Registering the Entry Points
- `module_init(...)`: Tells the kernel, "When someone runs `insmod` to load this module, execute hello_init first."
- `module_exit(...)`: Tells the kernel, "When someone runs `rmmod` to remove this module, execute hello_exit before taking it out of memory."


## 2. Update the Subdirectory Makefile
Open `linux-6.19/drivers/misc/Makefile` in a text editor and add the following line at the bottom. This tells the build system to look for your file:
```bash
vim linux-6.19/drivers/misc/Makefile
```
Add the following plaintext:
```Makefile
obj-$(CONFIG_HELLO_DRIVER) += hello.o
```
## 3. Update the Subdirectory Kconfig
Add the following plaintext to `linux-6.19/drivers/misc/Kconfig`
```
config HELLO_DRIVER
    tristate "Hello World Driver Support"
    default y
    help
      This option enables the custom Hello World driver.
```
```
config HELLO_DRIVER
```
- `HELLO_DRIVER` is defined as **Configuration Constants**
```
tristate "Hello World Driver Support"
```
- 解析：指定這個設定選項的類型與顯示名稱。
- **tristate**（三態）：這是 Linux 核心特有的類型，表示該驅動程式有三種編譯狀態可以選擇：

    - [*] **Built-in**（內建）：直接編譯進 Linux 核心映像檔（Kernel image）。
    - [M] **Module**（模組化）：編譯成獨立的核心模組（.ko 檔案），可在系統執行時動態載入或解除載入。
    - [ ] **Excluded**（不編譯）：完全不編譯此驅動程式。
- `Hello World Driver Support`：這是顯示在 menuconfig 圖形化選單介面上的文字描述，讓開發者知道這個選項的功能。
```
default y
```
- 解析：設定這個選項的預設值。
- 作用：**y** 代表預設為 **Built-in**（內建）。第一次開啟核心設定選單時，這個驅動程式預設為勾選納入核心的狀態。核心中常見的預設值還有 `n`（**預設不編譯**）或 `m`（**預設編譯為模組**）。
```
help
  This option enables the custom Hello World driver.
```
- 解析：說明文字（Help Text）。
- 作用：當開發者在設定選單中將游標移到 "`Hello World Driver Support`" 並按下 `Help` 鍵（通常是 `H` 鍵）時，畫面就會彈出這段提示訊息（「此選項用來啟用自訂的 Hello World 驅動程式」），協助開發者了解該選項用途。
- 注意：在 `Kconfig` 語法中，說明文字前方通常需要縮進（通常是兩個空格或一個 Tab），以表示它們屬於 `help` 的內容。
## 4. Configure and Build the Kernel
Generate the standard x86_64 configuration file (`.config`)
```bash
make x86_64_defconfig
```
Compile the kernel (adjust `-j` to the available CPU cores)
```
make -j$(nproc)
```
## 5. Use Git to Check Lastest Changes
```bash
git add -A
git status
```
output:
```bash
On branch master
Changes to be committed:
  (use "git restore --staged <file>..." to unstage)
	modified:   linux-6.19/drivers/misc/Kconfig
	modified:   linux-6.19/drivers/misc/Makefile
	new file:   linux-6.19/drivers/misc/hello.c
```

## 6. View the Boot Log inside QEMU
Boot the Kernel and Initramfs :
```bash
qemu-system-x86_64 \
   -kernel ./linux-6.19/arch/x86/boot/bzImage \
   -initrd ./initramfs.cpio.gz \
   -append "console=ttyS0" \
   -m 512
```
To verify that a Linux kernel module has loaded correctly and successfully printed a message to the kernel's ring buffer.
```bash
dmesg | grep "Hello, World!"
```
- `dmesg`: This utility prints the message buffer of the kernel, containing messages from device drivers and the kernel itself.
- `| (Pipe)`: This operator takes the output from dmesg and passes it as input to the next command.
- `grep "Hello, World!"`: This searches the passed input for the specific string **"Hello, World!"** and displays any lines that contain it.
