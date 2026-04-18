# TODO do AlmaOS

Este arquivo resume o que ainda precisa ser fechado para o sistema continuar evoluindo.
O foco e manter o boot, o kernel e a camada grafica com contratos claros.

## Estado atual

- O stage2 carrega `kernel.bin` para `0x1200:0x0000` e faz far-jump para la.
- O kernel agora entra em modo protegido 32-bit, instala GDT flat, IDT com 48 vetores,
  remapeia o PIC para `0x20-0x2F`, programa o PIT a ~100 Hz e habilita teclado via IRQ1.
- A saida usa memoria VGA text mode direta (`0xB8000`, 80x25) com printf basico.
- Um shell interativo aceita comandos: `help`, `clear`, `mem`, `ticks`, `reboot`.
- A biblioteca `gfx` foi clonada em [src/gfx](src/gfx) e possui backends CPU/GPU (Linux)
  e um novo backend bare-metal em [src/gfx/os/](src/gfx/os/) com API 2D e VGA.
- O stage2 continua em modo real 16-bit com OpenWatcom, cobrindo FAT12, disco e mapa E820.
- O demo de protected mode em [protected/src/main.asm](protected/src/main.asm) continua separado.

## 1. Base do sistema

- [x] Fechar o contrato do boot: stage1 → stage2 → kernel.bin.
- [x] Definir `kernel.bin` como caminho oficial; `protect.bin` e demo separado.
- [x] Padronizar argumentos de boot: drive, mapa de memoria, modo de video e ponteiros far.
- [x] Criar tratamento unico de erro: `panic`, `kassert` e log minimo.
- [x] Separar claramente o que e demo do que e runtime real.
- [x] Entrar em modo protegido 32-bit com GDT flat e far jump do bootstrap.

## 2. Memoria

- [x] Ler e guardar o mapa de memoria E820.
- [x] Criar alocador bump para o kernel (stage2 valida heap contra E820).
- [x] Reservar regioes fixas para stack, heap, buffers de I/O e framebuffer.
- [x] Definir contratos de near/far e evitar overflow de 64 KB.
- [ ] Criar alocador de paginas simples para modo protegido.
- [ ] Implementar paginacao basica (identity mapping pelo menos ate 1 MB).

## 3. Entrada, tempo e depuracao

- [x] Implementar teclado com buffer e leitura nao bloqueante (IRQ1, scancode set 1).
- [x] Programar o PIT canal 0 a ~100 Hz e expor ticks do sistema.
- [x] Adicionar PIC remapeado, IDT completa e rotinas de interrupcao (ISR 0-47).
- [x] Criar `panic`, `kassert` e `klog` para depuracao.
- [ ] Adicionar suporte a Caps Lock e teclas especiais (setas, F1-F12, Delete).
- [ ] Implementar um timer de alta resolucao ou sleep baseado em ticks.
- [ ] Adicionar serial port (COM1) como saida alternativa de log.

## 4. Disco e arquivos

- [x] Camada de disco robusta no stage2 (retry, CHS, LBA).
- [x] FAT12 funcional para carregar arquivos no boot.
- [ ] Implementar leitura de disco em modo protegido (FDC via I/O ports ou ATA PIO).
- [ ] Portar a API FAT para o kernel 32-bit.
- [ ] Definir como recursos serao carregados do disco: fontes, sprites, configuracoes.
- [ ] Escolher formato de assets: raw, BMP, TGA ou formato proprio.
- [ ] Definir nomenclatura e contrato da pasta de assets no disco.

## 5. API grafica

A `gfx` foi clonada para `src/gfx/` e organizada como biblioteca local em C.

### Estrutura no repo

- `src/gfx/include/` — API publica (`gfx.h`, `gfx_math.h`).
- `src/gfx/cpu/` — Backend CPU com framebuffer e rasterizador.
- `src/gfx/gpu/` — Backend GPU/OpenGL (Linux/X11).
- `src/gfx/os/` — Backend bare-metal para AlmaOS:
  - `gfx2d.h` / `gfx2d.c` — API 2D: `gfx2d_init`, `gfx2d_clear`, `gfx2d_put_pixel`,
    `gfx2d_fill_rect`, `gfx2d_draw_line`, `gfx2d_blit`, `gfx2d_draw_text`, `gfx2d_present`.
  - `vga_backend.h` / `vga_backend.c` — Acesso direto a VGA mode 13h e paleta DAC.
- `src/gfx/src/` — Codigo compartilhado (math, stubs, tinyobj).
- `src/gfx/tests/` — Testes e programas de validacao.

### O que ja existe

- [x] API 2D definida em `gfx2d.h` com contexto explicito.
- [x] Primitivas: pixel, retangulo, linha (Bresenham), blit, texto com fonte 8x8.
- [x] Double buffering via backbuffer.
- [x] Backend VGA com acesso direto ao framebuffer 0xA0000 e paleta DAC.
- [x] Fonte bitmap 8x8 embutida (ASCII 32-126, 95 glifos).

### O que falta

- [ ] Implementar trampolim de modo real para BIOS `int 10h` (trocar modo de video).
- [ ] Ativar VGA mode 13h (320x200, 256 cores) a partir do kernel.
- [ ] Integrar `gfx2d` no kernel principal como modulo opcional.
- [ ] Implementar clipping robusto para todas as primitivas.
- [ ] Adicionar suporte a VBE/VESA para resolucoes maiores.
- [ ] Criar carregador de sprites e bitmaps a partir de arquivos no disco.
- [ ] Testar e validar o rasterizador de triangulos do backend CPU.

### Contrato minimo

O kernel nao chama rotinas de video dispersas. Toda saida visual passa por:
- `vga.c` para modo texto (shell, panic, log).
- `gfx2d` para modo grafico (quando ativado).

### Ordem de implementacao recomendada

1. ~~Definir API publica minima.~~ ✓
2. ~~Implementar backend VGA text mode.~~ ✓
3. ~~Criar primitivas de pixel, retangulo, linha.~~ ✓
4. ~~Adicionar texto com fonte bitmap 8x8.~~ ✓
5. Implementar trampolim real-mode e ativar mode 13h.
6. Integrar blit, sprites e carregamento de recursos.
7. Sair do modo de teste para integrar ao kernel principal.

## 6. Recursos graficos

- [ ] Definir como os recursos vao ser empacotados no disco.
- [ ] Criar loader para bitmap, fonte e sprite (requer disco no kernel).
- [ ] Implementar cache simples para recursos carregados.
- [ ] Definir nomenclatura e contrato da pasta de assets.

## 7. Runtime do sistema

- [x] Criar shell minimo com prompt, leitura de teclado e execucao de comandos.
- [x] Comandos basicos: `help`, `clear`, `mem`, `ticks`, `reboot`.
- [ ] Adicionar comando para listar arquivos do disco (requer FAT no kernel).
- [ ] Adicionar comando para trocar modo de video (requer trampolim).
- [ ] Planejar multitarefa somente depois do kernel estar estavel.
- [ ] Documentar a ordem correta de prioridade para evitar trabalho espalhado.

## 8. Build e infraestrutura

- [x] Makefile do kernel compila bootstrap 16-bit + C 32-bit com GCC/NASM/LD.
- [x] Imagem de floppy FAT12 com stage1 + stage2 + kernel.bin.
- [x] CMake wrapping os targets do Makefile com CTest.
- [ ] Adicionar target `make gfx-test` para compilar e rodar testes do gfx no host.
- [ ] Adicionar CI para build automatico (GCC -m32 + NASM + OpenWatcom).
- [ ] Adicionar target `make run-graphics` para testar modo grafico no QEMU.

## 9. Criterio de pronto

O projeto passa de fase quando existir:

- [x] Boot previsivel (stage1 → stage2 → kernel em modo protegido).
- [x] Kernel carregado de forma consistente com IDT, PIC e PIT.
- [x] Memoria base e interrupcoes sob controle.
- [x] API grafica escolhida e isolada (gfx2d no repo, backend VGA definido).
- [ ] Recursos carregados por uma rota unica (disco no kernel).
- [x] Fluxo de depuracao que nao dependa de suposicao (panic, assert, klog, shell).
