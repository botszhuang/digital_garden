---
title: "General Purpose Input/Output (GPIO) driver"
auther: "Botsz"
date: 2026-05-21
---
# General Purpose Input/Output (GPIO) Driver

This is a try for me to comfortable with memory mapping, hardware registers, and the Linux device model without being overwhelmingly complex.



## Makefile
```makefile
# complie the source(gpio_driver.c) to built as a loadable kernel modules (outpur:gpio_driver.o).
obj-m += gpio_driver.o

KDIR ?= /lib/modules/$(shell uname -r)/build

# Use the built-in rules to compile the kernel module.
all:
	make -C $(KDIR) M=$(PWD) modules

# Clean the build files.
clean:
	make -C $(KDIR) M=$(PWD) clean
```

