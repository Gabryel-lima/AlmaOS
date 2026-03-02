#include "stdio.h"
#include "stdbool.h"
#include "x86.h"

/**
 *  printf_number - Função auxiliar para formatar e imprimir números em printf.
 *
 *  @param argp: Um ponteiro para o próximo argumento variável a ser processado.
 *  @param length: O comprimento do argumento (curto, longo, etc.).
 *  @param sign: Indica se o número é com sinal ou sem sinal.
 *  @param radix: A base numérica para formatação (10 para decimal, 16 para hexadecimal, etc.).
 *
 *  Esta função extrai o número do argumento variável com base no comprimento e sinal, converte-o para uma string e a exibe no console.
 *  Ela retorna um ponteiro atualizado para o próximo argumento variável após processar o número.
 */
static int* printf_number(int* argp, int length, bool sign, int radix);

void putc(char c) {
    x86_Video_WriteCharTeletype(c, 0);
}

void puts(const char* str){
    while(*str) {
        putc(*str++);
    }
}

void puts_f(const char far* str) {
    while(*str) {
        putc(*str++);
    }
}

#define PRINTF_STATE_NORMAL         0   /* Estado normal, processando caracteres comuns */
#define PRINTF_STATE_LENGTH         1   /* Estado de processamento de comprimento */
#define PRINTF_STATE_LENGTH_SHORT   2   /* Estado de processamento de comprimento curto */
#define PRINTF_STATE_LENGTH_LONG    3   /* Estado de processamento de comprimento longo */
#define PRINTF_STATE_SPEC           4   /* Estado de processamento de especificador */

#define PRINTF_LENGTH_DEFAULT       0   /* Comprimento padrão */
#define PRINTF_LENGTH_SHORT_SHORT   1   /* Comprimento curto curto */
#define PRINTF_LENGTH_SHORT         2   /* Comprimento curto */
#define PRINTF_LENGTH_LONG          3   /* Comprimento longo */
#define PRINTF_LENGTH_LONG_LONG     4   /* Comprimento longo longo */

void _cdecl printf(const char* fmt, ...) {
    // argp aponta para o primeiro argumento variável (após fmt)
    int* argp = (int*)&fmt;
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;

    // avançar argp para o primeiro argumento variável
    argp++;

    // processar a string de formato
    while (*fmt) {
        switch (state) {
            case PRINTF_STATE_NORMAL:
                switch (*fmt) {
                    case '%':   
                        state = PRINTF_STATE_LENGTH;
                        break;
                    default:    
                        putc(*fmt);
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                switch (*fmt) {
                    case 'h':   
                        length = PRINTF_LENGTH_SHORT;
                        state = PRINTF_STATE_LENGTH_SHORT;
                        break;
                    case 'l':   
                        length = PRINTF_LENGTH_LONG;
                        state = PRINTF_STATE_LENGTH_LONG;
                        break;
                    default:    
                        goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (*fmt == 'h') {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (*fmt == 'l') {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*fmt) {
                    case 'c':   
                        putc((char)*argp);
                        argp++;
                        break;

                    case 's':   
                        if (length == PRINTF_LENGTH_LONG || length == PRINTF_LENGTH_LONG_LONG) {
                            puts_f(*(const char far**)argp);
                            argp += 2;
                        }
                        else {
                            puts(*(const char**)argp);
                            argp++;
                        }
                        break;

                    case '%':   
                        putc('%');
                        break;

                    case 'd':
                    case 'i':   
                        radix = 10; sign = true;
                        argp = printf_number(argp, length, sign, radix);
                        break;

                    case 'u':   
                        radix = 10; sign = false;
                        argp = printf_number(argp, length, sign, radix);
                        break;

                    case 'X':
                    case 'x':
                    case 'p':   
                        radix = 16; sign = false;
                        argp = printf_number(argp, length, sign, radix);
                        break;

                    case 'o':   
                        radix = 8; sign = false;        
                        argp = printf_number(argp, length, sign, radix);
                        break;

                    // outros especificadores de formato podem ser implementados aqui
                    default:    
                        break;
                }

                // resetar estado e variáveis para o próximo especificador
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                break;
        }
        fmt++;
    }
}

const char g_HexChars[] = "0123456789abcdef";

static int* printf_number(int* argp, int length, bool sign, int radix) {
    // buffer para armazenar a representação em string do número
    char buffer[32];
    unsigned long long number;
    int number_sign = 1;
    int pos = 0;

    // extrair o número do argumento variável com base no comprimento e sinal
    switch (length){
        case PRINTF_LENGTH_SHORT_SHORT:
        case PRINTF_LENGTH_SHORT:
        case PRINTF_LENGTH_DEFAULT:
            if (sign) {
                int n = *argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            }
            else {
                number = *(unsigned int*)argp;
            }
            argp++;
            break;

        case PRINTF_LENGTH_LONG:
            if (sign) {
                long int n = *(long int*)argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            }
            else {
                number = *(unsigned long int*)argp;
            }
            argp += 2;
            break;

        case PRINTF_LENGTH_LONG_LONG:
            if (sign) {
                long long int n = *(long long int*)argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            }
            else {
                number = *(unsigned long long int*)argp;
            }
            argp += 4;
            break;
    }

    // converter o número para string com base no radix ASCII
    do {
        uint32_t rem;
        x86_div64_32(number, radix, &number, &rem);
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // adicionar sinal negativo se necessário
    if (sign && number_sign < 0)
        buffer[pos++] = '-';

    // imprimir o número em ordem reversa
    while (--pos >= 0)
        putc(buffer[pos]);

    return argp;
}
