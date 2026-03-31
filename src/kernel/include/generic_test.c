#include "../../bootloader/stage2/stdint.h"
#include "../../bootloader/stage2/stddef.h"
#include "../../bootloader/stage2/utility.h"

/** @brief Tipos de ponteiro/índice genéricos.
 *  @author Gabriel-lima
 *  @date 2024-04-30
 *  @file generic_test.c
 *  Definidos como aliases para os tipos de tamanho apropriados (ex: uintptr_t, ptrdiff_t, size_t).
 *  Isso permite que as operações genéricas sejam aplicadas a ponteiros e índices sem precisar de
 *  macros separados.
 *  @note Esses tipos são usados para operações genéricas em ponteiros e índices, permitindo que as macros genéricas sejam aplicadas a eles sem precisar de casos separados.
 */

typedef size_t usize;
typedef ptrdiff_t isize;
typedef uintptr_t uptr;

#include "../../kernel/include/generic_inst.h"

int main(void) {
    uint32_t a = 5, b = 7;
    uint32_t m = min(a, b);

    double x = 1.2, y = 3.4;
    double M = max(x, y);

    uint32_t arr[3] = {1,2,3};
    uint32_t s = array_sum(arr, 3);
    isize idx = array_find(arr, 3, 2);

    uint32_t p = 1, q = 2;
    swap(p, q);

    return (m==5 && (M>3.3) && s==6 && idx==1 && p==2 && q==1) ? 0 : 1;
}
