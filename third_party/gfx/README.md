# gfx (nucleo portatil) vendorizado

Copia do subconjunto portatil do projeto
[gfx](https://github.com/Gabryel-lima/gfx), embutida aqui para o kernel poder
desenhar sem reescrever um rasterizador do zero.

- Origem: https://github.com/Gabryel-lima/gfx
- Commit: `1ce47cbf9f3008dd57c644a6195183d78055f486`
- Sincronizado em: 2026-09-06

## O que foi copiado, e por que so isso

| Arquivo | Papel |
| --- | --- |
| `include/gfx_math.h` | Vec2/Vec3/Vec4/Mat4 e helpers de min/max/clamp |
| `include/gfx_raster.h` | `Framebuffer` generico e o rasterizador de triangulos |
| `core/gfx_math.c` | Implementacao da matematica |
| `core/gfx_raster.c` | Rasterizador por software com z-buffer |
| `core/gfx_framebuffer.c` | `gfx_fb_set_pixel` e `gfx_fb_clear` |

Exatamente esses cinco arquivos formam o alvo `GFX_CORE_FREESTANDING_CHECK`
do CMake do gfx, que os compila com `-ffreestanding` justamente para provar
que nao dependem de sistema operacional. O resto do gfx (`gfx_mesh.c`,
`tinyobj_loader`, a fachada de backends, o caminho X11/OpenGL) precisa de
`stdio`, `malloc` ou de Linux, e por isso fica de fora.

Eles compilam aqui com o mesmo toolchain do kernel — `gcc -m32 -ffreestanding
-nostdinc` — resolvendo `<stdint.h>` e `<stddef.h>` contra os headers do
proprio kernel em `src/kernel/include`.

## Como atualizar

Copie os mesmos cinco arquivos do gfx, atualize o commit acima e rode
`make smoke`. Nao edite esta copia: correcoes vao no gfx e voltam por aqui,
senao as duas arvores divergem em silencio.

## Onde a ponte fica

O codigo especifico do AlmaOS que liga isto ao hardware esta em
`src/kernel/gfx_bridge.c` — ele monta um `Framebuffer` do gfx sobre o modo de
video ativo (`src/kernel/video.c`) e faz o blit para o formato do hardware.
