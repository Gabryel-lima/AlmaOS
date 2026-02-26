# simple Makefile for AlmaOS project

ASM=nasm
SRC_DIR=src
BUILD_DIR=build

# default target
all: $(BUILD_DIR)/main_floppy.img
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/main_floppy.img: $(BUILD_DIR)/main.bin
	cp $(BUILD_DIR)/main.bin $(BUILD_DIR)/main_floppy.img
	truncate -s 1440k $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main.bin: $(SRC_DIR)/main.asm | $(BUILD_DIR)
	$(ASM) $(SRC_DIR)/main.asm -f bin -o $(BUILD_DIR)/main.bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(BUILD_DIR)/main_floppy.img
	qemu-system-i386 -fda $(BUILD_DIR)/main_floppy.img

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Usage:"
	@echo "  make        - build the floppy image"
	@echo "  make run    - run the floppy image in QEMU"
	@echo "  make clean  - clean the build directory"
