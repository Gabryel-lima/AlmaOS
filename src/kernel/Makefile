# ASM
ASM?=nasm
ASMFLAGS?=-f bin

SOURCES_ASM = $(wildcard *.asm)
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_ASM))

.PHONY: all always clean

all: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel/asm/main.o
	cp $< $@

$(BUILD_DIR)/kernel/asm/%.o: %.asm always
	$(ASM) $(ASMFLAGS) $< -o $@

always:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/kernel
	mkdir -p $(BUILD_DIR)/kernel/asm

clean:
	rm -rf $(BUILD_DIR)/kernel
	rm -rf "*.err"
