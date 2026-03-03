#pragma once

#include "stdint.h"

/** Função que copia uma quantidade especificada de bytes de uma área de memória para outra
 *  
 * @param dst: Um ponteiro para a área de destino onde os bytes serão copiados.
 * @param src: Um ponteiro para a área de origem de onde os bytes serão copiados.
 * @param num: O número de bytes a serem copiados.
 * @return: Um ponteiro para a área de destino (dst).
*/
void far* memcpy(void far* dst, const void far* src, uint16_t num);
/** Função que preenche uma área de memória com um valor especificado
 * 
 * @param ptr: Um ponteiro para a área de memória a ser preenchida.
 * @param value: O valor a ser preenchido.
 * @param num: O número de bytes a serem preenchidos.
 * @return: Um ponteiro para a área de memória (ptr).
 */
void far* memset(void far* ptr, int value, uint16_t num);
/** Função que compara duas áreas de memória
 * 
 * @param ptr1: Um ponteiro para a primeira área de memória.
 * @param ptr2: Um ponteiro para a segunda área de memória.
 * @param num: O número de bytes a serem comparados.
 * @return: 0 se as áreas forem iguais, um valor diferente de 0 caso contrário.
 */
int memcmp(const void far* ptr1, const void far* ptr2, uint16_t num);
