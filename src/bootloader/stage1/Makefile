BUILD_DIR?=build/
ASM?=nasm

.PHONY: all clean

all: stage1

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: boot.asm
	$(ASM) -f bin $< -o $@

clean:
	rm -rf $(BUILD_DIR)/stage1.bin
	rm -rf "*.err"
