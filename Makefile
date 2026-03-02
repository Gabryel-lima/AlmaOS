# simple Makefile for AlmaOS project

# Variables
ASM=nasm
SRC_DIR=src
BUILD_DIR=build

# Tools for 16-bit code (bootloader)
CC16=/usr/bin/watcom/binl/wcc
LD16=/usr/bin/watcom/binl/wlink

# Compila o fat.c e cria o diretório de build se necessário
FAT_SRC := tools/fat/fat.c
FAT_BIN := $(BUILD_DIR)/fat

# Configurações do subprojeto
STAGE1_DIR=$(SRC_DIR)/bootloader/stage1
STAGE2_DIR=$(SRC_DIR)/bootloader/stage2
KERNEL_DIR=$(SRC_DIR)/kernel

# Arquivos de saída
# Note que passamos o caminho absoluto para o sub-make para que ele saiba onde salvar
ABS_BUILD_DIR=$(shell pwd)/$(BUILD_DIR)

# Declare phony targets to avoid conflitos com arquivos de mesmo nome
.PHONY: all bootloader kernel fat run debug clean help stage1 stage2

#
# Default target: build a floppy FAT12 image containing bootloader + kernel
#
FLOPPY := $(BUILD_DIR)/floppy.img
all: $(FLOPPY) fat

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $@

# Regra para compilar o stage1 no sub-diretório
stage1: $(BUILD_DIR)
	$(MAKE) -C $(STAGE1_DIR) BUILD_DIR=$(ABS_BUILD_DIR)

# Regra para compilar o stage2 no sub-diretório
stage2: $(BUILD_DIR)
	$(MAKE) -C $(STAGE2_DIR) BUILD_DIR=$(ABS_BUILD_DIR)

# Regra para compilar o kernel no sub-diretório
kernel: $(BUILD_DIR)
	$(MAKE) -C $(KERNEL_DIR) BUILD_DIR=$(ABS_BUILD_DIR)

# Compilação da imagem de floppy combinando tudo
$(FLOPPY): stage1 stage2 kernel
	dd if=/dev/zero bs=512 count=2880 of=$@
	mkfs.fat -F12 -n "ALMAOS" $@
	mcopy -i $@ $(BUILD_DIR)/stage2.bin ::/stage2.bin
	mcopy -i $@ $(BUILD_DIR)/kernel.bin ::/kernel.bin
	mcopy -i $@ "test.txt" ::/test.txt
	dd if=$(BUILD_DIR)/stage1.bin conv=notrunc of=$@

# separate targets for bootloader and kernel
bootloader: stage1 stage2

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
	rm -rf $(STAGE1_DIR)/"*.err"
	rm -rf $(STAGE2_DIR)/"*.err"
	rm -rf $(KERNEL_DIR)/"*.err"

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
