# Generics e testes de tipos (kernel/include)

Este documento descreve a implementação de primitivas genéricas usadas no kernel (min, max, clamp, swap, array_sum, array_find), como os "generics" são instanciados e como usar os arquivos de teste/instanciação existentes.

Arquivos relevantes
- generic.h — macros que geram as funções estáticas inline por tipo (DEFINE_MIN, DEFINE_MAX, DEFINE_SWAP, DEFINE_ARRAY_SUM, etc.).
- generic_inst.h — lista `KERNEL_TYPES(...)`, instancia `DEFINE_ALL` para cada tipo e fornece wrappers em C11 `_Generic` que permitem chamar `min(a,b)` sem passar explicitamente o tipo.
- generic_test.c — exemplo de uso/validação; define `usize`, `isize`, `uptr` e inclui `generic_inst.h` para testar as operações.

Resumo da implementação

- `generic.h` contém as macros `DEFINE_*` que implementam as funções concretas para um tipo `T` (por exemplo `min_uint32_t`, `array_sum_uint32_t`). Essas funções são `static inline` e usam nomes com sufixo de tipo.
- `generic_inst.h` declara a X-macro `KERNEL_TYPES(X)` com a lista canônica de tipos do kernel e expande `KERNEL_TYPES(DEFINE_ALL)` para gerar as funções para cada tipo. Em seguida, cria wrappers amigáveis usando C11 `_Generic` para mapear chamadas como `min(a, b)` para `min_<tipo>(a,b)` com base no tipo de `a`.

Como usar (diretrizes)

- Inclua `generic_inst.h` apenas em arquivos de implementação (.c). Não inclua em headers públicos — macros podem colidir e expandir nomes de protótipos.
- Se for necessário declarar protótipos em headers públicos, proteja-os de expansão de macro (por exemplo, `#undef min` antes de declarar o protótipo e redefina se necessário no .c).
- Antes de incluir `generic_inst.h` em um .c, assegure que os aliases genéricos (`usize`, `isize`, `uptr`) estejam definidos (veja `generic_test.c` para exemplo).

Exemplo mínimo (padrão usado em `generic_test.c`):

```c
#include "../../bootloader/stage2/stdint.h"
#include "../../bootloader/stage2/stddef.h"

typedef size_t usize;
typedef ptrdiff_t isize;
typedef uintptr_t uptr;

#include "generic_inst.h"

uint32_t a = 5, b = 7;
uint32_t m = min(a, b);        // usa _Generic para escolher min_uint32_t

double x = 1.2, y = 3.4;
double M = max(x, y);          // usa min/max para floats

uint32_t arr[3] = {1,2,3};
uint32_t s = array_sum(arr, 3);
isize idx = array_find(arr, 3, 2);

uint32_t p = 1, q = 2;
swap(p, q);                    // macro passa os endereços: swap_type(&(p), &(q))
```

Comportamento e limitações importantes

- `_Generic` seleciona a função com base no tipo da expressão fornecida (tipicamente o primeiro argumento usado nas macros definidas em `generic_inst.h`). Se `a` e `b` têm tipos diferentes, a seleção segue o tipo de `a` — tome cuidado com conversões implícitas.
- Para arrays e buscas, `generic_inst.h` usa tipos de ponteiro no `_Generic` (por exemplo `uint32_t*`) para mapear `array_sum(arr, n)` para a versão correta.
- `swap(a,b)` espera variáveis (não expressões) porque a macro passa `&(a)` e `&(b)` — passar literais ou resultados de expressões falhará.
- Há um `default` nas macros `_Generic` que mapeia para `uint32_t` quando o tipo não é reconhecido — é útil como fallback, mas prefira declarar/usar explicitamente os tipos suportados.

Como estender

- Para adicionar um tipo novo ao conjunto usado pelo kernel, insira-o em `KERNEL_TYPES(X)` em `generic_inst.h` (por exemplo, `X(my_type)`) e assegure-se de que `DEFINE_ALL(my_type)` seja compatível ou ajuste as macros `DEFINE_*` conforme necessário.
- Se um tipo precisa apenas de subconjunto de operações, instancie manualmente as macros desejadas (ex.: `DEFINE_SWAP(uptr); DEFINE_MIN(uptr);`).

Possível implementação futura

- Separar a camada de _wrappers_ `_Generic` e a camada de geração de funções (`DEFINE_ALL`) em headers diferentes: manter `generic.h` (geração) como "private" e expor apenas um header leve com `_Generic` para inclusão controlada.
- Adicionar casos de teste automáticos na árvore de build (ex.: alvo `make test` ou pequena suíte que compile e execute `generic_test.c` em ambiente de desenvolvimento).

Ver também
- README principal do projeto: [README.md](../../../README.md)
- Código de referência: [generic.h](generic.h), [generic_inst.h](generic_inst.h), [generic_test.c](generic_test.c)

Autor: Gabriel-lima — documentação auxiliar para os headers genéricos do kernel.
