
BUILD_DIR := build

dtc -I dts -O dtb -o $(BUILD_DIR)/hardware.dtb hardware.dts


#qemu-system-aarch64 \
#    -M virt \
#    -m 512M \
#    -kernel ../0_kernel/alpine-standard-3.23.4-x86_64.iso \
#    -initrd rootfs.cpio.gz \
#    -dtb ./hardware.dtb \
#    -nographic \
#    -append "console=ttyAMA0"

#    