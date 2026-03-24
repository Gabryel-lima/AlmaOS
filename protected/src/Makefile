ASM := nasm
SRC := main.asm

.PHONY: all clean

all: $(BUILD_DIR)/protected-mode.bin

$(BUILD_DIR)/protected-mode.bin: $(SRC)
	mkdir -p $(BUILD_DIR)
	$(ASM) -f bin $(SRC) -o $(BUILD_DIR)/protected-mode.bin

clean:
	rm -f $(BUILD_DIR)/protected-mode.bin
