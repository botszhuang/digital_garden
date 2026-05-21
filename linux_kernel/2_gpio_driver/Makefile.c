# Makefile for GPIO driver
# This Makefile compiles the gpio_driver.c file into a kernel module.
# Usage:
#   make        - to build the kernel module
#   make clean  - to clean the build files  
# Author: Botsz HUANG
# Date: 2024-05-21

# 'gpio_driver.c' : the source file for the kernel module
# 'gpio_driver.o' : the object file for the kernel module.
# 'obj-m' : to built as a loadable kernel modules.
# '-m' :  module.
# '+=' : to add the module to the list of modules

obj-m += gpio_driver.o

# KDIR : Kernel Directory
# ?= : to set the value of a variable if it is not already set.
# uname -r : to get the current kernel version.
# $(shell ...) : to execute the command and get the output.

KDIR ?= /lib/modules/$(shell uname -r)/build

# Use the built-in rules to compile the kernel module.
all:
	make -C $(KDIR) M=$(PWD) modules

# Clean the build files.
clean:
	make -C $(KDIR) M=$(PWD) clean