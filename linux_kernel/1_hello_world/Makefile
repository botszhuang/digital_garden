MODULE_NAME := hello
PWD := $(shell pwd)
BUILD_DIR   := $(PWD)/build
 
KDIR ?= /lib/modules/$(shell uname -r)/build
 
obj-m += $(MODULE_NAME).o

all:
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	cp *.c $(BUILD_DIR)
	cp Makefile $(BUILD_DIR)
	make -C $(KDIR) M=$(BUILD_DIR) modules

clean:
	make -C $(KDIR) M=$(BUILD_DIR) clean
	rm -rf $(BUILD_DIR)