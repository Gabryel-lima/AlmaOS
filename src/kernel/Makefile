# Kernel Makefile - compila bootstrap 16-bit + kernel C 32-bit

ASM      ?= nasm
CC       ?= gcc
LD       ?= ld
OBJCOPY  ?= objcopy

CFLAGS   := -m32 -std=c11 -ffreestanding -nostdinc -nostdlib -fno-builtin \
            -fno-stack-protector -fno-pie -Wall -Wextra -O2 -Iinclude
LDFLAGS  := -m elf_i386 -T linker.ld --no-warn-rwx-segments

# Arquivos fonte
BOOT_SRC    := boot.asm
ENTRY_SRC   := entry.asm
ISR_SRC     := isr_stubs.asm
C_SOURCES   := kernel_main.c vga.c idt.c pic.c pit.c keyboard.c panic.c shell.c string.c

# Artefatos intermediarios
BOOT_BIN    := $(BUILD_DIR)/kernel/boot.bin
ENTRY_OBJ   := $(BUILD_DIR)/kernel/asm/entry.o
ISR_OBJ     := $(BUILD_DIR)/kernel/asm/isr_stubs.o
C_OBJECTS   := $(patsubst %.c,$(BUILD_DIR)/kernel/c/%.o,$(C_SOURCES))
KERNEL_ELF  := $(BUILD_DIR)/kernel/kernel_c.elf
KERNEL_CBIN := $(BUILD_DIR)/kernel/kernel_c.bin

.PHONY: all always clean

all: $(BUILD_DIR)/kernel.bin

# Bootstrap 16-bit (flat binary, 512 bytes)
$(BOOT_BIN): $(BOOT_SRC) always
	$(ASM) -f bin $< -o $@

# Assembly ELF32 objects
$(ENTRY_OBJ): $(ENTRY_SRC) always
	$(ASM) -f elf32 $< -o $@

$(ISR_OBJ): $(ISR_SRC) always
	$(ASM) -f elf32 $< -o $@

# C objects
$(BUILD_DIR)/kernel/c/%.o: %.c always
	$(CC) $(CFLAGS) -c $< -o $@

# Link ELF
$(KERNEL_ELF): $(ENTRY_OBJ) $(ISR_OBJ) $(C_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Extrai flat binary do ELF
$(KERNEL_CBIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Kernel final = boot.bin (512 bytes) + kernel_c.bin
$(BUILD_DIR)/kernel.bin: $(BOOT_BIN) $(KERNEL_CBIN)
	cat $^ > $@

always:
	@mkdir -p $(BUILD_DIR)/kernel
	@mkdir -p $(BUILD_DIR)/kernel/asm
	@mkdir -p $(BUILD_DIR)/kernel/c

clean:
	rm -rf $(BUILD_DIR)/kernel
