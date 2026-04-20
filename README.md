# AlmaOS

AlmaOS é um sistema operacional x86 de 16-bit simples, desenvolvido como um projeto de aprendizado seguindo o tutorial [OS do zero (nanobyte)](https://youtube.com/playlist?list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN).

## 🚀 Requisitos de Instalação

Para compilar e rodar este projeto no Linux (Ubuntu/Debian), você precisará das seguintes ferramentas:

```bash
sudo apt update
sudo apt install make okteta nasm qemu-system-x86 mtools dosfstools bochs bochs-sdl bochsbios vgabios
```

### Compilador C (16-bit)
Além das ferramentas acima, é necessário o **Open Watcom v2**. Ele é essencial para compilar código C destinado a ambientes de 16 bits (como o modo real do x86), garantindo que o código gerado seja compatível com as limitações de memória e segmentação do bootloader e kernel.

Você pode baixar a versão mais recente em: [Open Watcom v2 Tags](https://github.com/open-watcom/open-watcom-v2/releases/tag/2021-02-04-Build/open-watcom-2_0-c-linux-x64)

A instalação é feita com os comandos:

```bash
chmod +x open-watcom-2_0-c-linux-x64
sudo ./open-watcom-2_0-c-linux-x64
```

- O `okteta` é opcional, mas recomendado para inspecionar a imagem do disco gerada.

### Detalhes das Ferramentas:
- **NASM**: O montador (assembler) usado para converter código Assembly em binários.
- **GCC**: Compilador C usado para o kernel 32-bit (modo protegido, freestanding, `-m32 -std=c11`).
- **Open Watcom v2**: Compilador C/C++ focado em sistemas legados e desenvolvimento de baixo nível (16/32-bit). Usado para o bootloader stage2.
- **QEMU**: Emulador para testar o sistema operacional.
- **mtools & dosfstools**: Usados para criar e manipular imagens de disco FAT12.
- **Bochs**: Emulador de PC IA-32 (x86) útil para depuração avançada.
- **Make**: Para automatizar o processo de build.

## 🛠️ Como Compilar

Para compilar o bootloader e o kernel e gerar a imagem do disco (floppy):

```bash
make
```

Isso criará o diretório `build/` contendo o arquivo `floppy.img`.
A imagem padrão carrega `stage1.bin`, `stage2.bin` e `kernel.bin`; o alvo `protected-mode` continua disponível como demo separada.

### Build com CMake

Se preferir usar CMake como entrada principal sem abandonar o Makefile, configure um diretório separado:

```bash
cmake -S . -B build-cmake
cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
```

O projeto CMake apenas orquestra os alvos existentes do Makefile, então os dois fluxos permanecem equivalentes.

## ⚙️ Configuração do VS Code (IntelliSense)

O projeto possui dois ambientes de compilação distintos:
- **Bootloader (stage2)**: compilado com **Open Watcom** (16-bit, modo real).
- **Kernel**: compilado com **GCC** (32-bit, modo protegido, `-m32 -std=c11 -ffreestanding`).

Cada ambiente usa extensões de linguagem diferentes. O Watcom utiliza palavras-chave como `far`, `near`, `_cdecl` e `__interrupt`, enquanto o kernel GCC usa `__attribute__((packed))`, `__asm__ volatile(...)` e `static inline`. Uma configuração única de IntelliSense não reconheceria as duas ao mesmo tempo.

### Configuração unificada (recomendada)

O arquivo [.vscode/c_cpp_properties.json](.vscode/c_cpp_properties.json) inclui três perfis. O primeiro, **"AlmaOS"**, é o perfil padrão e funciona para ambos os ambientes: utiliza o GCC como compilador (o que permite ao IntelliSense entender `__attribute__` e `__asm__`) e define as palavras-chave do Watcom como macros vazias (o que evita erros falsos no código do bootloader).

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

### Alternando entre perfis

Para alternar entre os perfis, clique no nome da configuração ativa na barra de status do VS Code (canto inferior direito, ao lado do ícone do C/C++) e selecione o perfil desejado:

| Perfil | Quando usar |
|---|---|
| **AlmaOS** | Perfil padrão — funciona para kernel e bootloader ao mesmo tempo. |
| **Watcom (stage2 16-bit)** | Quando quiser IntelliSense estrito do Watcom para o bootloader. |
| **Kernel (GCC 32-bit)** | Quando quiser IntelliSense puro do GCC para o kernel. |

### settings.json

O arquivo [.vscode/settings.json](.vscode/settings.json) garante que o IntelliSense use o engine padrão (não o Tag Parser):

```json
{
    "C_Cpp.intelliSenseEngine": "default"
}
```

### Por que esses erros apareciam?

O VS Code C/C++ IntelliSense aplica **uma única configuração** para todos os arquivos abertos. Quando o perfil ativo era o Watcom, o engine não reconhecia extensões do GCC usadas no kernel:

- `__attribute__((packed))` → `expected a ';'`
- `__asm__ volatile(...)` → `identifier "__asm__" is undefined`
- Tipos definidos com `__attribute__` falhavam em cascata (`idt_entry_t`, `interrupt_frame_t`, etc.)

O perfil **"AlmaOS"** resolve isso usando o GCC como compilador para o IntelliSense (que entende nativamente essas extensões) e definindo as palavras-chave do Watcom como macros vazias.

**Vantagem:** Isso evita erros falsos tanto no código do kernel (extensões GCC) quanto no código do bootloader (palavras-chave Watcom), sem necessidade de trocar perfis manualmente.

## ⚙️ Configuração do tasks.json para Debug
Para configurar o `tasks.json` para usar o Watcom durante a depuração, você pode criar ou editar o arquivo [.vscode/tasks.json](.vscode/tasks.json) com o seguinte conteúdo:

```json
{
    "tasks": [
        {
            "type": "cppbuild",
            "label": "C/C++: Watcom build active file",
            "command": "/usr/bin/watcom/binl/wcc",
            "args": [
                "-d2",
                "-ml",
                "-za99",
                "${file}",
                "-fo=${fileDirname}/${fileBasenameNoExtension}.obj"
            ],
            "options": {
                "cwd": "${fileDirname}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "detail": "Task generated by Debugger."
        },
        {
            "label": "make clean && make && make run",
            "type": "shell",
            "command": "make clean && make && make run",
            "group": "test"
        }
    ],
    "version": "2.0.0"
}
```

**Vantagem:** Isso permite que você compile e execute o projeto diretamente do VS Code usando as tarefas definidas, garantindo que o processo de build utilize o Watcom corretamente.

## ⚙️ Configuração do launch.json para Depuração
Para configurar o `launch.json` para depurar o AlmaOS usando o QEMU, você pode criar ou editar o arquivo [.vscode/launch.json](.vscode/launch.json) com o seguinte conteúdo:

- Ainda não funciona a depuração porque a floppy.img não é um executável, talvez 
seja necessário criar um kernel.elf para o depurador reconhecer, ou usar um script de inicialização do GDB para configurar os registradores corretamente.

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Run AlmaOS (QEMU)",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/floppy.img",
            "cwd": "${workspaceFolder}",
            "preLaunchTask": "make clean && make && make run",
            "externalConsole": false
        }
    ]
}
```

**Vantagem:** Isso permite que você inicie a depuração do AlmaOS diretamente do VS Code, executando o QEMU com a imagem do disco gerada, facilitando o processo de teste e depuração.

## 💻 Como Executar

### Usando QEMU (Recomendado)
Para rodar rapidamente no QEMU:

```bash
make run
```

### Usando Bochs (Depuração)
Para rodar no Bochs usando o arquivo de configuração local:

```bash
make debug
```

## 📂 Estrutura do Projeto

- `src/bootloader/`: Código do setor de boot (stage1 em Assembly, stage2 em C/Assembly com Watcom 16-bit).
- `src/kernel/`: Código principal do kernel (C/Assembly com GCC 32-bit, modo protegido).
  - `src/kernel/root/`: Comandos do shell com assinatura portável `int fn(int argc, char **argv)`. Cada arquivo é compilado como objeto independente e registrado na tabela de dispatch do shell.
  - `src/kernel/tests/`: Utilitários de teste compilados no host: primitivas genéricas (`generic.h`, `generic_inst.h`) e validação de scancodes de teclado (`test_keyboard.c`).
- `src/gfx/`: Biblioteca gráfica auxiliar (CPU rasterizer e GPU via OpenGL/X11).
- `build/`: Arquivos binários gerados e imagem final.
- `.vscode/`: Configurações do VS Code (IntelliSense, tasks, launch).
- `Makefile`: Script de automação do build.
- `bochs_config`: Configurações para o emulador Bochs.

### Shell e comandos

O shell usa uma tabela de dispatch (`cmd_table[]`) em vez de uma cadeia `if/else`. Cada entrada associa um nome de comando a um ponteiro de função com assinatura `int fn(int argc, char **argv)`.

**Builtins** (estáticos em `shell.c`, dependem do estado interno do kernel):
- `help`, `clear`, `mem`, `ticks`, `reboot`

**Comandos externos** (compilados em `root/`, declarados via header próprio):
- `echo` — `root/echo.c` / `root/echo.h`; aceita `-n`/`--no-newline` (suprime quebra de linha) e `-h`/`--help`.

Para adicionar um novo comando:
1. Crie `src/kernel/root/meu_cmd.c` com `int meu_cmd(int argc, char **argv)`.
2. Crie `src/kernel/root/meu_cmd.h` com a declaração.
3. Adicione `root/meu_cmd.c` em `C_SOURCES` no `src/kernel/Makefile`.
4. Inclua o header em `shell.c` e adicione a entrada na `cmd_table[]`.

### Driver de teclado

O driver PS/2 (`keyboard.c`) suporta dois layouts e três camadas de entrada:

| Layout | Constante |
|---|---|
| US QWERTY | `KB_LAYOUT_US` |
| Brasileiro ABNT2 | `KB_LAYOUT_ABNT2` (padrão) |

Camadas de mapeamento por layout:
- **Normal** — tecla pressionada sem modificador.
- **Shift** — com Left Shift ou Right Shift pressionado.
- **AltGr** — com Right Alt pressionado.

Modificadores rastreados:
- `Shift` (Left/Right) — ativo enquanto pressionado.
- `Caps Lock` — toggle; inverte caixa apenas para letras (a–z / A–Z).
- `AltGr` (Right Alt) — ativo enquanto pressionado.
- Prefixo `0xE0` — rastreado para distinguir Right Alt de Left Alt.

Para trocar o layout em tempo de execução:

```c
keyboard_set_layout(KB_LAYOUT_US);    // US QWERTY
keyboard_set_layout(KB_LAYOUT_ABNT2); // Brasileiro (padrão)
kb_layout_t atual = keyboard_get_layout();
```

> **Nota:** Dead keys do ABNT2 (´ acento agudo, ~ til) são mapeados diretamente ao caractere base; combinação de acento não é suportada. Os caracteres ç/Ç usam encoding ISO-8859-1 (0xE7/0xC7), compatível com a tabela de glifos CP850 do VGA.

### Parser de opções (`getopt`)

O módulo `getopt.c` / `include/getopt.h` implementa análise de opções compatível com POSIX para os builtins do shell:

- `getopt()` — opções curtas (`-n`, `-v`, etc.).
- `getopt_long()` — opções longas no estilo GNU (`--no-newline`, `--help=valor`, etc.).

```c
#include "include/getopt.h"

int meu_cmd(int argc, char **argv) {
    int opt;
    optind = 1; /* reinicializar antes de cada parse */
    while ((opt = getopt(argc, argv, "hn:")) != -1) {
        switch (opt) {
        case 'h': /* exibe ajuda */  break;
        case 'n': /* usa optarg */   break;
        }
    }
    return 0;
}
```

> Reinicialize `optind = 1` antes de cada novo `argv[]`.

### Primitivas genéricas (`tests/generic`)

Os arquivos `src/kernel/tests/generic.h` e `generic_inst.h` fornecem funções type-safe geradas por macro:

| Operação | Descrição |
|---|---|
| `min(a, b)` | menor entre dois valores |
| `max(a, b)` | maior entre dois valores |
| `clamp(v, lo, hi)` | limita `v` ao intervalo `[lo, hi]` |
| `swap(a, b)` | troca os valores de `a` e `b` |
| `array_fill(arr, n, val)` | preenche array com `val` |
| `array_sum(arr, n)` | soma todos os elementos |
| `array_find(arr, n, val)` | índice da primeira ocorrência ou -1 |

Tipos suportados: `uint8_t`…`uint64_t`, `int8_t`…`int64_t`, `float32_t`, `float64_t`, `long_double`, `uptr`, `isize`, `usize`.

Inclua `generic_inst.h` apenas em arquivos `.c` (nunca em headers públicos). Consulte [src/kernel/tests/Generics.md](src/kernel/tests/Generics.md) para detalhes e exemplos.

---
Desenvolvido para fins educacionais.

# Licença
Este projeto está licenciado sob a Licença MIT. Veja o arquivo [LICENSE](./LICENSE) para mais detalhes.
