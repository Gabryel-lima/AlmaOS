# simple Makefile for AlmaOS project

# Variables
ASM=nasm
SRC_DIR=src
BUILD_DIR=build

# Compila o fat.c e cria o diretório de build se necessário
FAT_SRC := tools/fat/fat.c
FAT_BIN := $(BUILD_DIR)/fat

# Declare phony targets to avoid conflitos com arquivos de mesmo nome
.PHONY: all bootloader kernel fat run debug clean help 

#
# Default target: build a floppy FAT12 image containing bootloader + kernel
#
FLOPPY := $(BUILD_DIR)/floppy.img
all: $(FLOPPY) fat # build the FAT utility as well (optional, for debugging)

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
	dd if=/dev/zero bs=512 count=2880 of=$@
	mkfs.fat -F12 -n "ALMAOS" $@
	mcopy -i $@ $(BUILD_DIR)/kernel.bin ::/kernel.bin
	mcopy -i $@ "test.txt" ::/test.txt # copy a test file to the image (optional)
	dd if=$(BUILD_DIR)/boot.bin conv=notrunc of=$@

# separate targets for bootloader and kernel
bootloader: $(BUILD_DIR)/boot.bin
kernel: $(BUILD_DIR)/kernel.bin

# build the FAT utility (optional, for debugging)
fat: $(FAT_BIN)

# build the FAT utility from its source code
$(FAT_BIN): $(FAT_SRC) | $(BUILD_DIR)
	gcc -O2 -Wall $< -o $@

# run in QEMU
run: $(FLOPPY)
	qemu-system-i386 -fda $<

# run in Bochs
debug: $(FLOPPY)
	bochs -f bochs_config

# housekeeping
clean:
	rm -rf $(BUILD_DIR)

# help message
help:
	@echo "Usage:"
	@echo "  make          - build the floppy image"
	@echo "  make kernel   - build the kernel binary"
	@echo "  make bootloader - build the bootloader binary"
	@echo "  make fat      - build the FAT utility"
	@echo "  make run      - run the floppy image in QEMU"
	@echo "  make debug    - run the floppy image in Bochs"
	@echo "  make clean    - clean the build directory"
	@echo "  make help     - display this help message"
