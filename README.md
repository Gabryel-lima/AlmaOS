# AlmaOS

AlmaOS is a simple 16-bit x86 operating system developed as a learning project following the "OS from scratch (nanobyte)" tutorial.

- Português (pt-BR): [README](README_PT.md)

## 🚀 Installation Requirements

To build and run this project on Linux (Ubuntu/Debian) you will need the following tools:

```bash
sudo apt update
sudo apt install make okteta nasm qemu-system-x86 mtools dosfstools bochs bochs-sdl bochsbios vgabios
```

### 16-bit C compiler

In addition to the tools above, the project requires **Open Watcom v2**. This compiler is necessary to build C code targeting 16-bit real mode (stage2 bootloader).

Download Open Watcom v2 (example release):

https://github.com/open-watcom/open-watcom-v2/releases/tag/2021-02-04-Build/open-watcom-2_0-c-linux-x64

Install it with:

```bash
chmod +x open-watcom-2_0-c-linux-x64
sudo ./open-watcom-2_0-c-linux-x64
```

- `okteta` is optional but recommended to inspect the generated disk image.

### Tools overview

- **NASM**: Assembler for assembly sources.
- **GCC**: C compiler used for the 32-bit kernel (protected mode, freestanding).
- **Open Watcom v2**: C compiler for the bootloader stage2 (16-bit / real mode).
- **QEMU**: Emulator to run the OS image.
- **mtools & dosfstools**: Create and manipulate FAT12 disk images.
- **Bochs**: IA-32 emulator useful for advanced debugging.
- **Make**: Build automation.

## 🛠️ How to Build

To build the bootloader and kernel and generate the floppy image:

```bash
make
```

This will create the `build/` directory containing `floppy.img`. The default image includes `stage1.bin`, `stage2.bin` and `kernel.bin`. There is also a `protected-mode` demo target.

### Build with CMake

If you prefer CMake:

```bash
cmake -S . -B build-cmake
cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
```

The CMake setup orchestrates the existing Makefile targets, so both build flows are equivalent.

## ⚙️ VS Code (IntelliSense) Configuration

The repository includes multiple IntelliSense profiles to support both the Watcom bootloader and the GCC kernel. The default profile, **"AlmaOS"**, uses GCC for parsing and defines Watcom-specific keywords as empty macros to avoid false positives across both codebases.

An example `.vscode/c_cpp_properties.json` profile set (already included in the repo):

```json
{
  "configurations": [
    {
      "name": "AlmaOS",
      "includePath": [
        "${workspaceFolder}/src/kernel",
        "${workspaceFolder}/src/kernel/include",
        "${workspaceFolder}/src/bootloader/stage2"
      ],
      "defines": [
        "_cdecl=",
        "far=",
        "near=",
        "huge=",
        "__far=",
        "__near=",
        "__huge=",
        "__interrupt=",
        "__watcall="
      ],
      "compilerPath": "/usr/bin/gcc",
      "cStandard": "c11",
      "intelliSenseMode": "linux-gcc-x86",
      "compilerArgs": [
        "-m32",
        "-ffreestanding",
        "-nostdinc"
      ]
    },
    {
      "name": "Watcom (stage2 16-bit)",
      "includePath": [
        "${workspaceFolder}/src/bootloader/stage2"
      ],
      "defines": [
        "_cdecl=",
        "far=",
        "near=",
        "huge=",
        "__far=",
        "__near=",
        "__huge=",
        "__interrupt=",
        "__watcall="
      ],
      "compilerPath": "/usr/bin/watcom/binl/wcc",
      "cStandard": "c99",
      "cppStandard": "gnu++17",
      "intelliSenseMode": "linux-gcc-x64",
      "compilerArgs": [
        "-4 -d3 -s -wx -ms -zl -zq -nt=STAGE2_CODE"
      ]
    },
    {
      "name": "Kernel (GCC 32-bit)",
      "includePath": [
        "${workspaceFolder}/src/kernel",
        "${workspaceFolder}/src/kernel/include"
      ],
      "defines": [],
      "compilerPath": "/usr/bin/gcc",
      "cStandard": "c11",
      "intelliSenseMode": "linux-gcc-x86",
      "compilerArgs": [
        "-m32",
        "-ffreestanding",
        "-nostdinc",
        "-nostdlib",
        "-fno-builtin",
        "-fno-stack-protector",
        "-fno-pie"
      ]
    }
  ],
  "version": 4
}
```

To change the active profile, click the C/C++ configuration name in the status bar (bottom-right) and choose the profile.

Add this to `.vscode/settings.json` to prefer the default IntelliSense engine:

```json
{
  "C_Cpp.intelliSenseEngine": "default"
}
```

### Why those errors appeared

When the Watcom profile was active, the IntelliSense parser did not understand GCC-specific attributes and inline assembly used by the kernel (for example `__attribute__((packed))` or `__asm__ volatile(...)`). The **"AlmaOS"** profile resolves these false positives by using GCC for parsing and defining Watcom keywords as empty macros.

## ⚙️ tasks.json and launch.json examples

Example `tasks.json` entries to build with Watcom and to run the project's make targets are included in the repo (see `.vscode/tasks.json`).

Example `launch.json` to launch QEMU is also provided; note that debugging a floppy image may require a proper `kernel.elf` or a GDB init script to set registers correctly.

## 💻 How to Run

### QEMU (recommended)

```bash
make run
```

### Bochs (debugging)

```bash
make debug
```

## 📂 Project Structure

- `src/bootloader/`: Boot sector and stage2 (stage1 assembly, stage2 C/assembly with Watcom).
- `src/kernel/`: Kernel sources (C/assembly, GCC 32-bit protected mode).
  - `src/kernel/root/`: Shell commands with portable signature `int fn(int argc, char **argv)`.
  - `src/kernel/tests/`: Host-side test utilities.
- `src/gfx/`: Graphics helpers and examples (rasterizer, GPU helpers).
- `build/`: Generated binaries and final floppy image.
- `.vscode/`: VS Code configuration files.
- `Makefile`: Build automation.
- `bochs_config`: Bochs configuration file.

## Shell and commands

The shell uses a `cmd_table[]` dispatch table instead of chaining `if/else`. Each entry maps a command name to a function pointer with signature `int fn(int argc, char **argv)`.

Builtins (static in `shell.c`): `help`, `clear`, `mem`, `ticks`, `reboot`.

External commands are implemented under `src/kernel/root/` and declared via a small header; for example, `echo` is in `root/echo.c` and supports `-n/--no-newline` and `-h/--help`.

To add a new command:
1. Create `src/kernel/root/my_cmd.c` with `int my_cmd(int argc, char **argv)`.
2. Create `src/kernel/root/my_cmd.h` with the declaration.
3. Add `root/my_cmd.c` to `C_SOURCES` in `src/kernel/Makefile`.
4. Include the header in `shell.c` and add an entry to `cmd_table[]`.

## Keyboard driver

The PS/2 keyboard driver supports two layouts and three mapping layers:

| Layout | Constant |
|---|---|
| US QWERTY | `KB_LAYOUT_US` |
| Brazilian ABNT2 | `KB_LAYOUT_ABNT2` (default) |

Mapping layers: Normal, Shift, AltGr (Right Alt). Modifiers tracked: Shift, Caps Lock (toggle), AltGr, and the 0xE0 prefix for distinguishing some keys.

Example API:

```c
keyboard_set_layout(KB_LAYOUT_US);
keyboard_set_layout(KB_LAYOUT_ABNT2);
kb_layout_t current = keyboard_get_layout();
```

Note: Dead keys on ABNT2 are mapped directly to the base character; composed accents are not supported. ç/Ç use ISO-8859-1 encoding (0xE7 / 0xC7), compatible with VGA CP850 glyphs.

## Option parser (`getopt`)

The project provides a POSIX-compatible `getopt` implementation (`src/kernel/getopt.c` / `include/getopt.h`) supporting short and long options. Reset `optind = 1` before each parse.

## Generic primitives (tests)

Host-side test helpers in `src/kernel/tests/generic.h` and `generic_inst.h` provide type-safe macro-generated functions like `min`, `max`, `clamp`, `swap`, `array_fill`, `array_sum`, `array_find` for various integer and floating types. See `src/kernel/tests/Generics.md` for details.

---

Developed for educational purposes.

# License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
