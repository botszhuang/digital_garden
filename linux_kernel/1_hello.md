---
title: "Hello World"
auther: "Botsz"
date: 2026-05-21
---
## Makefile
```makefile
# Build hello.o as a module
# hello.c --> hello.o --> hello.ko
obj-m += hello.o

KDIR ?= /lib/modules/$(shell uname -r)/build

# Use the built-in rules to compile the kernel module.
all:
	make -C $(KDIR) M=$(PWD) modules

# Clean the build files.
clean:
	make -C $(KDIR) M=$(PWD) clean
```
- `obj-m`: Compiles the target as a dynamically loadable kernel module (.ko).

## hello.c

`<linux/init.h>` is a critical header file used in Linux kernel development to define macros for module initialization and cleanup.
- `module_init(function_name)`: Registers the function to be called when the module is inserted (via `insmod`) or at boot time if built-in.
- `module_exit(function_name)`: Registers the function to be called when the module is removed (via `rmmod`).
- `__init`: A marker used for initialization functions. The kernel can free the memory used by these functions after they finish running to save RAM.
- `__exit`: A marker for cleanup functions, indicating they are only needed if the module is removed.

`<linux/module.h>` is a fundamental C header file required for writing and compiling Loadable Kernel Modules (LKMs) in the Linux kernel. It provides the necessary structures and macros to allow code to be dynamically loaded into or removed from the running kernel without needing a system reboot.

```C
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
- `printk()` is the kernel's equivalent of `printf`.
- `KERN_ALERT` is a log level macro used in the Linux kernel to define the priority of a message sent to `printk()`.

| Macro | Level | Meaning | Example Scenario |
| :--- | :--- | :--- | :--- |
| KERN_EMERG|0|System is unusable|Imminent crash / complete hardware failure |
| KERN_ALERT|1|Action must be taken immediately|Corrupted database or critical state breach |
| KERN_CRIT|2|Critical conditions|Hard device error (e.g., bad sector on system drive) |
| KERN_ERR|3|Error conditions|Driver failed to initialize a device |
| KERN_WARNING|4|Warning conditions|Something is wrong, but the system can keep running |
| KERN_NOTICE|5|Normal but significant|Standard security or state changes |
| KERN_INFO|6|Informational messages|Driver loaded successfully |
| KERN_DEBUG|7|Debug-level messages|Verbose troubleshooting output |

## RUN.sh
```bash
# insert Module
sudo insmod build/hello.ko

# Display the kernel ring buffer, filtered to the last 10 lines.
sudo dmesg | tail -n 10

# remove Module.
sudo rmmod hello
```
### Display the kernel ring buffer 
Because kernel modules run in kernel space, they cannot print text to the standard user terminal screen using printf. Instead, functions like printk() write to a special **internal memory buffer** called the **kernel ring buffer**. [Ref](./0_install_qemu.md)
- `insmod`: It is a trivial program to insert a module into the kernel
- `rmmod`: It is a trivial program to remove a module from the kernel. 
- `dmesg`: It prints the message buffer of the kernel, containing messages from device drivers and the kernel itself.

