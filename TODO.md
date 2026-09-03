# TODO do AlmaOS

Este arquivo resume o que ainda precisa ser fechado para o sistema continuar evoluindo.
O foco e manter o boot, o kernel e a camada grafica com contratos claros.

## Estado atual

- O stage2 carrega `kernel.bin` para `0x1200:0x0000` e faz far-jump para la.
- O kernel agora entra em modo protegido 32-bit, instala GDT flat, IDT com 48 vetores,
  remapeia o PIC para `0x20-0x2F`, programa o PIT a ~100 Hz e habilita teclado via IRQ1.
- A saida usa memoria VGA text mode direta (`0xB8000`, 80x25) com printf basico.
- Um shell interativo aceita comandos: `help`, `clear`, `mem`, `ticks`, `reboot`.
- A camada grafica ainda nao foi integrada: `gfx` (github.com/Gabryel-lima/gfx) e um
  repositorio externo, ainda nao vendorizado aqui. Ver secao 5.
- O stage2 continua em modo real 16-bit com OpenWatcom, cobrindo FAT12, disco e mapa E820.
- O demo de protected mode em [protected/src/main.asm](protected/src/main.asm) continua separado.
- O shell usa tabela de dispatch com tokenizador de argc/argv; comandos portaveis vivem em `src/kernel/root/`.

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
- [x] Implementar layout ABNT2 com tabelas completas (normal, shift, AltGr, tecla extra 0x73).
- [x] Adicionar Caps Lock (toggle), AltGr (Right Alt) e rastreamento de prefixo 0xE0.
- [x] Implementar `getopt()` e `getopt_long()` para parsing de opcoes em builtins (`getopt.c` / `getopt.h`).
- [ ] Adicionar suporte a teclas especiais como eventos distintos (setas, F1-F12, Delete).
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

**Estado real (corrigido):** `gfx` (github.com/Gabryel-lima/gfx) e um repositorio
separado, ainda **nao vendorizado** dentro do AlmaOS — nao existe `src/gfx/` neste
repo nem nunca existiu (sem historico de commit). Uma versao anterior deste TODO
descrevia um backend bare-metal (`gfx2d`, `vga_backend`) como se ja estivesse
pronto; isso nunca foi implementado. Esta secao documenta o estado real e o
gancho de integracao que ja existe hoje, para nao repetir o mesmo erro.

### O que ja existe hoje, do lado do AlmaOS

O contrato de handoff bootloader → kernel ja reserva espaco para um framebuffer,
mesmo sem nenhum backend grafico ligado nele:

- `src/bootloader/stage2/memdefs.h` reserva `0xA0000`–`0xBFFFF` para
  framebuffer/VGA (`MEMORY_FRAMEBUFFER_ADDR`, `MEMORY_FRAMEBUFFER_SIZE`).
- `src/bootloader/stage2/memory.h` / `memory.c` ja aceitam `framebuffer`,
  `framebufferWidth/Height/Pitch/Bpp/Flags` em `MEMORY_BootInfo_Init()`.
- `src/kernel/include/boot_info.h` espelha os mesmos campos em
  `boot_info_raw_t`, no endereco fixo `BOOT_INFO_ADDR = 0x60000`.
- **Porem**, a chamada real em `src/bootloader/stage2/main.c` passa todos os
  seis argumentos de framebuffer como `0` e `MEMORY_VIDEO_MODE_TEXT` — nenhuma
  chamada VBE/VESA existe ainda em lugar nenhum do stage2.

Ou seja: o contrato de dados existe e esta bem desenhado; falta so alguem
preenche-lo (ativar um modo de video) e alguem le-lo do lado do kernel.

### Do lado do `gfx` (repositorio externo)

O `gfx` foi redesenhado para separar um nucleo portatil (`include/` + `core/`,
sem dependencia de SO alem da libc padrao: matematica, rasterizador por
software, framebuffer generico `pixels/width/height/pitch`) de extras
especificos de Linux (`platform/linux/`: `/dev/fb0`, janela X11/OpenGL). O
nucleo (`gfx_math.c` + `gfx_raster.c` + `gfx_framebuffer.c`) ja e validado no
proprio `gfx` com um alvo de CMake (`GFX_CORE_FREESTANDING_CHECK`) que compila
essas tres fontes com `-ffreestanding`, exatamente para provar que dao para
embutir num kernel. Isso ainda **nao foi portado para dentro do AlmaOS** — e
so a base que torna essa integracao factivel sem reescrever um rasterizador do zero.

### O que falta para uma integracao real

- [ ] Implementar trampolim de modo real para BIOS `int 10h` (o kernel ja esta
      em modo protegido 32-bit e nao pode chamar a BIOS diretamente).
- [ ] Ativar um modo de video no stage2 — mais simples primeiro: VGA mode 13h
      (320x200, 256 cores, endereco fixo `0xA0000`, sem VBE); depois VBE/VESA
      para resolucoes maiores (o `boot_info` ja tem `fb_pitch`/`fb_bpp` para isso).
- [ ] Preencher de verdade os argumentos de framebuffer em
      `MEMORY_BootInfo_Init()` (hoje todos zero) em vez de so reservar o espaco.
- [ ] Copiar `include/` + `core/` do `gfx` para dentro do AlmaOS (ex.:
      `third_party/gfx/`) e compilar com o toolchain `-m32 -ffreestanding` do kernel.
- [ ] Escrever a ponte kernel-especifica: ler `boot_info()->framebuffer_far`
      via `far_to_flat()`, montar um `Framebuffer` do `gfx` (gfx_raster.h) em
      cima dele, e expor isso como um modulo opcional do kernel.
- [ ] So depois disso faz sentido pensar em sprites, fontes, blit — tudo isso
      ja existe no `gfx` como conceito (rasterizador + framebuffer), o trabalho
      aqui e so a ponte, nao reescrever a parte grafica.

### Contrato minimo (mantido)

O kernel nao chama rotinas de video dispersas. Toda saida visual passa por:
- `vga.c` para modo texto (shell, panic, log) — ja funciona hoje.
- Um modulo grafico opcional (a integrar) para modo grafico, construido sobre
  o nucleo do `gfx`, quando ativado.

## 6. Recursos graficos

- [ ] Definir como os recursos vao ser empacotados no disco.
- [ ] Criar loader para bitmap, fonte e sprite (requer disco no kernel).
- [ ] Implementar cache simples para recursos carregados.
- [ ] Definir nomenclatura e contrato da pasta de assets.

## 7. Runtime do sistema

- [x] Criar shell minimo com prompt, leitura de teclado e execucao de comandos.
- [x] Comandos basicos: `help`, `clear`, `mem`, `ticks`, `reboot`.
- [x] Refatorar shell para tabela de dispatch (`cmd_table[]`) com `int fn(int argc, char **argv)`.
- [x] Tokenizador de linha de comando (popula `argc`/`argv` a partir de `cmd_buf`).
- [x] Separar comandos portaveis em `root/` (cada arquivo compila como objeto independente).
- [x] Implementar `echo` como primeiro comando em `root/` (com flags `-n`/`--no-newline` e `-h`/`--help`).
- [x] Implementar primitivas genericas type-safe (`generic.h`, `generic_inst.h`) com testes no host.
- [ ] Adicionar comando `ls` para listar arquivos do disco (requer FAT no kernel).
- [ ] Adicionar comando `cat` para exibir conteudo de arquivo (requer FAT no kernel).
- [ ] Adicionar comando `mode` para trocar modo de video (requer trampolim real-mode).
- [ ] Adicionar historico de comandos (seta para cima/baixo).
- [ ] Adicionar autocompletar por Tab baseado na `cmd_table`.
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
- [ ] API grafica escolhida e isolada (gfx externo definido; integracao no kernel ainda por fazer — ver secao 5).
- [ ] Recursos carregados por uma rota unica (disco no kernel).
- [x] Fluxo de depuracao que nao dependa de suposicao (panic, assert, klog, shell).
