MODULE_NAME := gpio_driver
BUILD_DIR   := $(PWD)/build

# KDIR: Makefile for building the GPIO driver kernel module
KDIR ?= /lib/modules/$(shell uname -r)/build

# Cross-compilation settings
#ARCH ?= arm64
#CROSS_COMPILE ?= aarch64-linux-gnu-

obj-m += $(MODULE_NAME).o

all:
	@rm -rf $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)
	@make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) modules

clean:
	@make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) clean
	@rm -rf $(BUILD_DIR)