# simple Makefile for AlmaOS project

# Variables
ASM=nasm
SRC_DIR=src
BUILD_DIR=build

# Declare phony targets to avoid conflicts with files of the same name
.PHONY: all bootloader kernel run clean help

#
# Default target: build a floppy FAT12 image containing bootloader + kernel
#
FLOPPY := $(BUILD_DIR)/floppy.img
all: $(FLOPPY)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $@

# assemble bootloader and kernel separately
$(BUILD_DIR)/boot.bin: $(SRC_DIR)/bootloader/boot.asm | $(BUILD_DIR)
	$(ASM) -f bin $< -o $@

# assemble kernel (flat binary)
$(BUILD_DIR)/kernel.bin: $(SRC_DIR)/kernel/main.asm | $(BUILD_DIR)
	$(ASM) -f bin $< -o $@

# create a 1.44-MB FAT12 floppy image and copy the bootloader + kernel into it
$(FLOPPY): $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	dd if=/dev/zero bs=512 count=2880 of=$@        # 1.44-MB blank image
	mkfs.fat -F12 $@
	dd if=$(BUILD_DIR)/boot.bin conv=notrunc count=1 bs=512 of=$@
	mcopy -i $@ $(BUILD_DIR)/kernel.bin ::/kernel.bin

# separate targets for bootloader and kernel
bootloader: $(BUILD_DIR)/boot.bin
kernel: $(BUILD_DIR)/kernel.bin

# run in QEMU
run: $(FLOPPY)
	qemu-system-i386 -fda $<

# housekeeping
clean:
	rm -rf $(BUILD_DIR)

# help message
help:
	@echo "Usage:"
	@echo "  make          - build the floppy image"
	@echo "  make kernel   - build the kernel binary"
	@echo "  make bootloader - build the bootloader binary"
	@echo "  make run      - run the floppy image in QEMU"
	@echo "  make clean    - clean the build directory"
	@echo "  make help     - display this help message"
