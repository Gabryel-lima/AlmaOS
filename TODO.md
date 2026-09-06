# TODO do AlmaOS

Este arquivo resume o que ainda precisa ser fechado para o sistema continuar evoluindo.
O foco e manter o boot, o kernel e a camada grafica com contratos claros.

## Estado atual

- O stage2 carrega `kernel.bin` para `0x1200:0x0000` e faz far-jump para la.
- O kernel agora entra em modo protegido 32-bit, instala GDT flat, IDT com 48 vetores,
  remapeia o PIC para `0x20-0x2F`, programa o PIT a ~100 Hz e habilita teclado via IRQ1.
- A saida usa memoria VGA text mode direta (`0xB8000`, 80x25) com printf basico,
  espelhada na COM1 para depuracao e automacao.
- Um shell interativo aceita comandos: `help`, `clear`, `mem`, `ticks`, `reboot`,
  `echo`, `mode` e `gfxdemo`. Ele le do teclado ou da serial, o que chegar primeiro.
- A camada grafica esta integrada: o nucleo portatil do `gfx` vive em
  `third_party/gfx/` e a ponte com o hardware em `src/kernel/gfx_bridge.c`.
  O kernel troca de modo de video em runtime por um trampolim de modo real.
  Ver secao 5.
- O stage2 continua em modo real 16-bit com OpenWatcom, cobrindo FAT12, disco e mapa E820.
- Onde o OpenWatcom nao existe, `tools/devboot/devboot.asm` (so NASM) cumpre o mesmo
  contrato de handoff e permite rodar o kernel sob QEMU; `make smoke` automatiza isso.
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
- [x] Criar alocador de paginas simples para modo protegido (`src/kernel/kalloc.c`:
      alocador bump alinhado a pagina sobre a maior regiao utilizavel do E820 acima
      de 1 MiB). Nao libera bloco individual — e o suficiente enquanto as alocacoes
      sao poucas e vivem pelo resto do boot, como framebuffers e z-buffer.
- [ ] Implementar paginacao basica (identity mapping pelo menos ate 1 MB).
      Nada disso comecou. Vale notar que ate agora nao fez falta: sem paginacao,
      endereco linear e endereco fisico, e foi justamente isso que permitiu ao
      kernel escrever num framebuffer VBE acima de 1 MiB sem mapear nada.

## 3. Entrada, tempo e depuracao

- [x] Implementar teclado com buffer e leitura nao bloqueante (IRQ1, scancode set 1).
- [x] Programar o PIT canal 0 a ~100 Hz e expor ticks do sistema.
- [x] Adicionar PIC remapeado, IDT completa e rotinas de interrupcao (ISR 0-47).
- [x] Criar `panic`, `kassert` e `klog` para depuracao.
- [x] Implementar layout ABNT2 com tabelas completas (normal, shift, AltGr, tecla extra 0x73).
- [x] Adicionar Caps Lock (toggle), AltGr (Right Alt) e rastreamento de prefixo 0xE0.
- [x] Implementar `getopt()` e `getopt_long()` para parsing de opcoes em builtins (`getopt.c` / `getopt.h`).
- [x] Adicionar serial port (COM1) como saida alternativa de log
      (`src/kernel/serial.c`, 16550 a 38400 8N1, com teste de loopback para nao
      fingir que ha UART onde nao ha). `vga_putchar()` espelha cada caractere,
      entao o log existente virou capturavel sem um segundo printf a manter — e
      sobrevive a troca para modo grafico, quando a tela de texto some.
- [x] Aceitar entrada pela COM1 alem do teclado, para que uma sessao de shell
      possa ser roteirizada (`tools/devboot/smoke.sh`).
- [ ] Adicionar suporte a teclas especiais como eventos distintos (setas, F1-F12, Delete).
- [ ] Implementar um timer de alta resolucao ou sleep baseado em ticks.

## 4. Disco e arquivos

- [x] Camada de disco robusta no stage2 (retry, CHS, LBA).
- [x] FAT12 funcional para carregar arquivos no boot.
- [ ] Implementar leitura de disco em modo protegido (FDC via I/O ports ou ATA PIO).
- [ ] Portar a API FAT para o kernel 32-bit.
- [ ] Definir como recursos serao carregados do disco: fontes, sprites, configuracoes.
- [ ] Escolher formato de assets: raw, BMP, TGA ou formato proprio.
- [ ] Definir nomenclatura e contrato da pasta de assets no disco.

## 5. API grafica

**Estado: integrado.** O `gfx` (github.com/Gabryel-lima/gfx) deixou de ser so um
repositorio vizinho: o nucleo portatil dele esta vendorizado em `third_party/gfx/`
e compila com o mesmo toolchain do kernel. Uma versao anterior deste TODO
descrevia um backend bare-metal (`gfx2d`, `vga_backend`) que nunca existiu;
esta secao descreve o que ha de fato, com os arquivos.

### O caminho completo, hoje

1. **Trampolim de modo real** — `src/kernel/realmode.{asm,c,h}`. O kernel entra em
   modo protegido no boot e, com isso, perde acesso as rotinas da BIOS, que sao
   codigo de 16 bits esperando modo real. O trampolim faz PM32 -> RM16 -> PM32
   para chamadas pontuais.
2. **Selecao de modo de video** — `src/kernel/video.{c,h}`. Por cima do trampolim:
   VGA modo 13h (320x200, 8 bits, sempre disponivel) e VBE 2.0+ com framebuffer
   linear (resolucoes maiores e cor direta, quando a BIOS de video oferecer).
3. **Memoria** — `src/kernel/kalloc.{c,h}`. Um backbuffer de 640x480 em 32 bits
   pede 1,2 MiB; o heap que o stage2 reserva tem 128 KiB.
4. **FPU** — `src/kernel/fpu.{c,h}`. O rasterizador do gfx trabalha em ponto
   flutuante.
5. **Nucleo do gfx** — `third_party/gfx/` (matematica, framebuffer generico,
   rasterizador por software com z-buffer). Veja o README de la para a
   procedencia e como ressincronizar.
6. **Ponte** — `src/kernel/gfx_bridge.{c,h}`. Unico ponto do kernel que conhece
   os dois lados.
7. **Comandos** — `src/kernel/cmd_video.c`: `mode` e `gfxdemo`.

### Decisoes que valem registrar

- **Quem troca o modo e o kernel, nao o bootloader.** A versao anterior deste
  arquivo previa ligar um modo grafico no stage2. Isso teria custado a saida de
  texto do kernel (panic, log, shell) logo no boot. O stage2 continua entregando
  o controle em modo texto 80x25 — e agora descreve esse modo no `boot_info`, em
  vez de mandar seis zeros como se nao houvesse framebuffer nenhum.
- **A ponte converte, nao aponta.** O gfx desenha num backbuffer RGBA de 32 bits
  e a ponte converte para o formato do modo ativo. Apontar o gfx direto para o
  framebuffer do hardware seria mais rapido e errado: o QEMU reporta R@16 G@8 B@0
  (BGRX) enquanto o gfx produz `0xRRGGBBAA`, e no modo 13h nem ha 32 bits por pixel.
- **A paleta 3-3-2 do modo 13h nao e estetica, e aritmetica.** Com ela o indice de
  8 bits vira funcao pura da cor de 24 bits, e a conversao no `present` e
  truncamento — sem tabela de busca nem dithering.
- **Os descritores de 16 bits da GDT tem a base preenchida em runtime.** Eles
  precisam apontar para o stub do trampolim, cujo endereco tem um dono so
  (`REALMODE_STUB_ADDR`, em `include/realmode.h`). Fixar o valor em `boot.asm`
  criaria uma segunda fonte da verdade.

### O que falta aqui

- [ ] Carregar malhas de disco em vez de gerar geometria no proprio codigo
      (depende de FAT no kernel — secao 4).
- [ ] Fontes e sprites por cima do rasterizador (depende de recursos em disco —
      secao 6).
- [ ] Considerar apontar o gfx direto para o framebuffer nos modos de 32 bits,
      evitando a copia do `present`. So vale a pena depois de haver algo pesado
      o suficiente para o custo aparecer.

### Contrato minimo (mantido)

O kernel nao chama rotinas de video dispersas. Toda saida visual passa por:
- `vga.c` para modo texto (shell, panic, log);
- `gfx_bridge.c` para modo grafico, construido sobre o nucleo do `gfx`.

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
- [x] Adicionar comando `mode` para trocar modo de video (o trampolim de modo real
      existe agora; veja secao 5). Aceita `text`, `vga13` e `LARGURAxALTURA[xBPP]`.
- [x] Adicionar comando `gfxdemo`, que desenha um cubo com o rasterizador do gfx.
      A variante `gfxdemo <modo> still` desenha um quadro em angulo fixo e le o
      framebuffer do hardware de volta, resumindo cobertura e numero de cores —
      e o que torna o caminho grafico verificavel sem tela.
- [ ] Adicionar historico de comandos (seta para cima/baixo).
- [ ] Adicionar autocompletar por Tab baseado na `cmd_table`.
- [ ] Planejar multitarefa somente depois do kernel estar estavel.
- [ ] Documentar a ordem correta de prioridade para evitar trabalho espalhado.

## 8. Build e infraestrutura

- [x] Makefile do kernel compila bootstrap 16-bit + C 32-bit com GCC/NASM/LD.
- [x] Imagem de floppy FAT12 com stage1 + stage2 + kernel.bin.
- [x] CMake wrapping os targets do Makefile com CTest.
- [x] Adicionar target `make gfx-test` para compilar e rodar testes do gfx no host.
      Faz duas coisas: compila a copia vendorizada com `-ffreestanding` (provando que
      ela continua embutivel) e verifica no host os contratos de que a ponte depende
      — formato de cor, sentido do z-buffer, pitch em bytes e correcao perspectiva.
      Uma ressincronizacao com o gfx upstream pode mudar qualquer um deles sem
      quebrar nada la, e aqui quebraria a ponte inteira.
- [x] Adicionar carregador de desenvolvimento so-NASM (`tools/devboot/`) e o alvo
      `make smoke`, que sobe o kernel no QEMU sem tela, digita comandos pela COM1 e
      confere as respostas.
- [x] Adicionar CI para build automatico (`.github/workflows/kernel-ci.yml`).
      **Cobertura parcial, de proposito:** o OpenWatcom nao existe nos runners do
      GitHub, entao o CI cobre o kernel 32-bit, o gfx vendorizado, os testes de host
      e o boot sob QEMU pelo devboot. Uma regressao no stage2 ainda precisa de uma
      maquina com o OpenWatcom instalado.
- [x] Adicionar target para testar modo grafico no QEMU: `make smoke` inclui os
      casos `gfxdemo 320x200x32 still` e `gfxdemo vga13 still`, e `make run-devboot`
      abre a sessao interativa.

## 9. Criterio de pronto

O projeto passa de fase quando existir:

- [x] Boot previsivel (stage1 → stage2 → kernel em modo protegido).
- [x] Kernel carregado de forma consistente com IDT, PIC e PIT.
- [x] Memoria base e interrupcoes sob controle.
- [x] API grafica escolhida e isolada: nucleo do gfx vendorizado em `third_party/gfx/`,
      hardware isolado atras de `src/kernel/gfx_bridge.c` (ver secao 5).
- [ ] Recursos carregados por uma rota unica (disco no kernel).
- [x] Fluxo de depuracao que nao dependa de suposicao (panic, assert, klog, shell).
