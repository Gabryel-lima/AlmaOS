# ASM
ASM?=nasm
ASMFLAGS?=-f bin
CC16?=/usr/bin/watcom/binl/wcc
CFLAGS16?=-4 -d3 -s -wx -ms -zl -zq -nt=STAGE2_CODE
LD16?=/usr/bin/watcom/binl/wlink

SOURCES_C = $(wildcard *.c)
SOURCES_ASM = $(wildcard *.asm)
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/stage2/c/%.o, $(SOURCES_C))
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/stage2/asm/%.o, $(SOURCES_ASM))

.PHONY: all always clean

all: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(LD16) NAME $(BUILD_DIR)/stage2.bin FILE { $(OBJECTS_ASM) $(OBJECTS_C) } OPTION MAP=$(BUILD_DIR)/stage2.map FORMAT RAW BIN OPTION QUIET, NODEFAULTLIBS, START=entry, OFFSET=0, STACK=0x200

$(BUILD_DIR)/stage2/c/%.o: %.c always
	$(CC16) $(CFLAGS16) -fo=$@ $<

$(BUILD_DIR)/stage2/asm/%.o: %.asm always
	$(ASM) -f obj $< -o $@

always:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/stage2
	mkdir -p $(BUILD_DIR)/stage2/c
	mkdir -p $(BUILD_DIR)/stage2/asm

clean:
	rm -f $(BUILD_DIR)/$(TARGET)
	rm -rf "*.err"
