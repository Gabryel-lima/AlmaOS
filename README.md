# AlmaOS

AlmaOS é um sistema operacional x86 de 16-bit simples, desenvolvido como um projeto de aprendizado seguindo o tutorial [OS do zero (nanobyte)](https://youtube.com/playlist?list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN).

## 🚀 Requisitos de Instalação

Para compilar e rodar este projeto no Linux (Ubuntu/Debian), você precisará das seguintes ferramentas:

```bash
sudo apt update
sudo apt install make okteta nasm qemu-system-x86 mtools dosfstools bochs bochs-sdl bochsbios vgabios
```
- O `okteta` é opcional, mas recomendado para inspecionar a imagem do disco gerada.

### Detalhes das Ferramentas:
- **NASM**: O montador (assembler) usado para converter código Assembly em binários.
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
