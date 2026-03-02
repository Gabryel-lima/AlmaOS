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

- O `okteta` é opcional, mas recomendado para inspecionar a imagem do disco gerada.

### Detalhes das Ferramentas:
- **NASM**: O montador (assembler) usado para converter código Assembly em binários.
- **Open Watcom v2**: Compilador C/C++ focado em sistemas legados e desenvolvimento de baixo nível (16/32-bit).
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

## ⚙️ Configuração do VS Code (IntelliSense)

Para ter o preenchimento de código (IntelliSense) correto usando o **Watcom** em vez do GCC padrão, siga estes passos:

1. Pressione `Ctrl + Shift + P`.
2. Procure e selecione **C/C++: Edit Configurations (UI)** (ícone de engrenagem).
3. No painel que abrir, você pode criar ou editar um perfil para usar o compilador em `/usr/bin/watcom/binl/wcc`.
4. Certifique-se de que o `cStandard` está definido como `c99` e o `intelliSenseMode` como `linux-gcc-x64` (ou compatível).

Alternativamente, você pode criar o arquivo [.vscode/c_cpp_properties.json](.vscode/c_cpp_properties.json) com o seguinte conteúdo:

```json
{
    "configurations": [
        {
            "name": "Watcom",
            "includePath": [
                "${default}"
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
        }
    ],
    "version": 4
}
```

**Vantagem:** Isso evita que o VS Code aponte erros falsos relacionados a bibliotecas padrão do Linux/GCC que não existem no ambiente de 16-bit do AlmaOS, além de reconhecer corretamente as diretivas específicas do Watcom como `_cdecl`.

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

- `src/bootloader/`: Contém o código do setor de boot.
- `src/kernel/`: Contém o código principal do kernel.
- `build/`: Arquivos binários gerados e imagem final.
- `Makefile`: Script de automação do build.
- `bochs_config`: Configurações para o emulador Bochs.

---
Desenvolvido para fins educacionais.
