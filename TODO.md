# TODO do AlmaOS

Este arquivo resume o que ainda precisa ser fechado para o sistema continuar evoluindo.
O foco e manter o boot, o kernel e a futura camada grafica com contratos claros.

## Estado atual

- O stage2 agora cobre carregamento de arquivos, testes de FAT e coleta do mapa de memoria em [src/bootloader/stage2/main.c](src/bootloader/stage2/main.c).
- O fluxo de modo protegido/demo esta em [protected/src/main.asm](protected/src/main.asm).
- O kernel basico de teste esta em [src/kernel/main.asm](src/kernel/main.asm).

## 1. Base do sistema

- [x] Fechar o contrato do boot: stage1 -> stage2 -> payload principal.
- [x] Definir qual binario e o caminho oficial do sistema: `kernel.bin`, `protect.bin` ou um unico payload consolidado.
- [x] Padronizar os argumentos de boot: drive, mapa de memoria, modo de video e ponteiros far.
- [x] Criar tratamento unico de erro para boot, disco, FAT e carregamento.
- [x] Separar claramente o que e demo do que e runtime real.

## 2. Memoria

- [x] Ler e guardar o mapa de memoria da maquina ou do emulador.
- [x] Criar um alocador simples para o kernel.
- [x] Reservar regioes fixas para stack, heap, buffers de I/O e framebuffer.
- [x] Definir contratos de near/far e evitar overflow de 64 KB onde isso for uma limitacao real.

## 3. Entrada, tempo e depuracao

- [ ] Implementar teclado com buffer e leitura nao bloqueante.
- [ ] Programar o timer/PIT e expor ticks do sistema.
- [ ] Se o caminho final for protegido, adicionar PIC, IDT e rotina de interrupcoes.
- [ ] Criar `panic`, `assert` e um logger minimo para depuracao.

## 4. Disco e arquivos

- [ ] Terminar a camada de disco com leitura robusta e erros consistentes.
- [ ] Consolidar FAT em uma API de arquivo mais limpa.
- [ ] Definir como recursos serao carregados do disco: fontes, sprites, imagens e configuracoes.
- [ ] Escolher um formato simples para assets: raw, BMP, TGA ou um formato proprio.

## 5. API grafica: decisao obrigatoria

Antes de implementar menus, janelas, cursor, HUD ou sprites, a camada grafica precisa ser definida.
Nao vale espalhar chamadas de video pelo kernel inteiro.

Neste projeto, a direcao preferida e clonar a `gfx` para dentro do repositorio e seguir com ela como
uma biblioteca local em C, organizada de forma mais limpa por modulos, em vez de deixar a logica
grafica espalhada pelo kernel.

### Estrutura sugerida no repo

Uma divisao inicial que tende a ficar mais limpa aqui e esta:

- `src/gfx/include/` para a API publica.
- `src/gfx/core/` para tipos basicos, estado global e contratos comuns.
- `src/gfx/backend/` para a ponte com VGA, VBE, BIOS, framebuffer ou outro backend escolhido.
- `src/gfx/render/` para primitivas de desenho, texto, blit e composicao.
- `src/gfx/assets/` para carregamento e validacao de recursos visuais.
- `src/gfx/tests/` para testes ou programas de validacao fora do kernel.

Essa separacao ajuda a evitar um unico arquivo grande e deixa claro o que e API, o que e backend e o
que e logica de renderizacao.

### Responsabilidades da gfx

A `gfx` deve ser a unica porta de entrada para a camada visual do projeto. O kernel nao deve chamar
rotinas de video dispersas diretamente, exceto em adaptadores muito finos.

Responsabilidades centrais:

- inicializar o modo de video e expor o framebuffer;
- manter o estado do contexto grafico;
- desenhar primitivos simples;
- carregar e apresentar texto e recursos;
- esconder detalhes do backend usado em cada ambiente.

Responsabilidades que nao devem ficar na gfx:

- logica de shell;
- regras de boot;
- leitura de disco;
- gerenciamento geral de memoria do kernel;
- politica de multitarefa.

### Opcao A: backend externo ou do firmware

Usar uma API ja existente para abrir video no boot ou em um backend de teste.

Exemplos:

- BIOS `int 10h` / VBE para setar modo grafico em 16-bit.
- UEFI GOP se o projeto migrar para UEFI no futuro.
- SDL2 apenas como backend de teste no host ou no emulador, nao como dependencia do kernel final.
- VGA text mode ou framebuffer linear como etapa de arranque enquanto a API final nao existe.

### Opcao B: a nossa API `gfx` em C

Consolidar uma camada propria e manter o kernel falando apenas com ela.
Aqui esta a opcao que melhor encaixa no projeto: trazer a `gfx` para este repo e evoluir como lib
interna, com separacao clara entre API publica, renderer, backend e recursos.

Exemplo de contrato minimo:

- `gfx_init(...)`
- `gfx_set_mode(...)`
- `gfx_put_pixel(...)`
- `gfx_fill_rect(...)`
- `gfx_draw_line(...)`
- `gfx_blit(...)`
- `gfx_draw_text(...)`
- `gfx_present(...)`

Exemplo de uso:

```c
gfx_init(&fb);
gfx_clear(0x000000);
gfx_draw_text(8, 8, "AlmaOS", 0xFFFFFF);
gfx_fill_rect(16, 32, 120, 24, 0x0044AA);
gfx_present();
```

Se a escolha for `gfx`, ela tambem precisa definir:

- formato do framebuffer;
- largura, altura e pitch;
- bpp e ordem dos canais;
- clipping;
- double buffering;
- carregamento de recursos como bitmap, fonte e sprite.
- estrutura dos modulos da biblioteca, para nao virar um arquivo unico gigante.

### Contrato minimo da biblioteca

Para nao virar apenas um conjunto de funcoes soltas, a `gfx` deveria nascer com estes blocos:

- `gfx.h` como face publica da biblioteca;
- um contexto grafico explicito, passado por referencia quando fizer sentido;
- backend isolado por plataforma;
- renderizador independente do backend;
- carregadores de recursos separados por formato.

Exemplo de organizacao conceitual:

- `gfx_context` para estado atual;
- `gfx_surface` para buffers de desenho;
- `gfx_backend` para acesso ao video real;
- `gfx_font` e `gfx_sprite` para recursos carregados;
- `gfx_draw_*` para primitivas de renderizacao.

### Ordem de implementacao recomendada

1. Definir a API publica minima.
2. Implementar um backend simples de framebuffer ou VGA text mode.
3. Criar desenho de pixel, retangulo e linha.
4. Adicionar texto e fonte bitmap.
5. Integrar blit e sprites.
6. Sair do modo de teste para integrar ao kernel principal.

## 6. Recursos graficos

- [ ] Definir como os recursos vao ser empacotados no disco.
- [ ] Criar loader para bitmap, fonte e sprite.
- [ ] Implementar cache simples para recursos carregados.
- [ ] Definir nomeclatura e contrato da pasta de assets.

## 7. Runtime do sistema

- [ ] Criar shell ou prompt minimo.
- [ ] Expor comandos basicos para listar arquivos, abrir recursos e testar video.
- [ ] Planejar multitarefa somente depois do kernel estar estavel.
- [ ] Documentar a ordem correta de prioridade para evitar trabalho espalhado.

## 8. Criterio de pronto

O projeto passa de fase quando existir:

- boot previsivel;
- kernel carregado de forma consistente;
- memoria base e interrupcoes sob controle;
- API grafica escolhida e isolada;
- recursos carregados por uma rota unica;
- um fluxo de depuracao que nao dependa de suposicao.
