#pragma once

/** @file idt.h
 *  @brief Interrupt Descriptor Table para modo protegido 32-bit.
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note A IDT é uma tabela de 256 entradas, onde cada entrada é um descritor 
 *  de interrupção que define o handler a ser chamado para um vetor de interrupção específico.
 */

#include "kernel.h"

/** Numero total de entradas na IDT. */
#define IDT_ENTRIES 256

/** @brief Frame de interrupcao empilhado pelos stubs em isr_stubs.asm.
 *  @details Ordem: segment regs, pushad, isr_num, error_code, eip, cs, eflags.
 *  @param gs, fs, es, ds Registradores de segmento (empilhados manualmente pelos stubs).
 *  @param edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax Registradores gerais (empilhados por pushad).
 *  @param isr_num Numero do vetor de interrupcao (empilhado manualmente pelos stubs).
 *  @param error_code Codigo de erro (empilhado pelos stubs para algumas interrupcoes, ou 0).
 *  @param eip, cs, eflags Registradores de controle (empilhados automaticamente pela CPU).
 */
typedef struct __attribute__((packed)) {
    /* +0, +4, +8, +12: Registradores de segmento (empilhados manualmente pelos stubs) */
    uint32_t gs, fs, es, ds;
    /* +16, +20, +24, +28, +32, +36, +40, +44: Registradores gerais (empilhados por pushad) */
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    /* +48, +52: Numero do vetor de interrupcao e codigo de erro (empilhados manualmente pelos stubs) */
    uint32_t isr_num, error_code;
    /* +56, +60, +64: Registradores de controle (empilhados automaticamente pela CPU) */
    uint32_t eip, cs, eflags;
} interrupt_frame_t;

/** @brief Callback registrado para um vetor de interrupcao.
 *  @param frame Ponteiro para o frame contendo o estado da CPU no momento da interrupcao.
 */
typedef void (*isr_handler_t)(interrupt_frame_t *frame);

/** @brief Inicializa a IDT e carrega-a com LIDT.
 *  @note Deve ser chamada apenas uma vez durante a inicializacao do kernel,
 *  depois que o stack seguro para interrupcoes estiver configurado.
 */
void idt_init(void);

/** @brief Registra um handler C para um vetor especifico.
 *  @param vector Numero do vetor de interrupcao.
 *  @param handler Ponteiro para a funcao handler a ser registrada.
 */
void idt_register_handler(uint8_t vector, isr_handler_t handler);
