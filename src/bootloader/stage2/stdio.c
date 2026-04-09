#include "stdio.h"
#include "stdbool.h"
#include "x86.h"

/**
 *  Formata e imprime o próximo argumento numérico de printf.
 *
 *  @param argp: Ponteiro para o próximo argumento variável.
 *  @param length: Modificador de comprimento já decodificado.
 *  @param sign: Indica se o valor deve ser tratado como assinado.
 *  @param radix: Base usada na conversão numérica.
 *  @return: Ponteiro atualizado para o próximo argumento variável.
 */
static int* printf_number(int* argp, int length, bool sign, int radix);

void putc(char c) {
    x86_Video_WriteCharTeletype(c, 0);
}

static uint32_t g_Utf32State = 0; // Estado atual do decodificador UTF-8 (0 = esperando por um novo caractere, >0 = esperando por bytes de continuação)
static uint32_t g_Utf32Char = 0; // Caractere UTF-32 em construção (armazenado enquanto bytes de continuação são processados)

void putc_utf8(char c) {
    uint8_t b = (uint8_t)c;

    if (b < 0x80) {
        g_Utf32State = 0;
        putc(c);
    } else if (g_Utf32State == 0) {
        if ((b & 0xE0) == 0xC0) {
            g_Utf32Char = b & 0x1F;
            g_Utf32State = 1;
        } else if ((b & 0xF0) == 0xE0) {
            g_Utf32Char = b & 0x0F;
            g_Utf32State = 2;
        } else if ((b & 0xF8) == 0xF0) {
            g_Utf32Char = b & 0x07;
            g_Utf32State = 3;
        }
    } else {
        g_Utf32Char = (g_Utf32Char << 6) | (b & 0x3F);
        g_Utf32State--;
        if (g_Utf32State == 0) {
            putwc((uint16_t)g_Utf32Char);
        }
    }
}

void puts(const char* str){
    while(*str) {
        putc_utf8(*str++);
    }
}

void puts_f(const char far* str) {
    while(*str) {
        putc_utf8(*str++);
    }
}

void putwc(uint16_t c) {
    if (c < 0x80) {
        putc((char)c);
    } else {
        // Mapeamento básico para caracteres acentuados comuns (UTF-16 -> CP437)
        // Isso pode ser expandido conforme necessário
        switch (c) {
            case 0x00C4: putc(0x8E); break; // Ä
            case 0x00C5: putc(0x8F); break; // Å
            case 0x00C7: putc(0x80); break; // Ç
            case 0x00C9: putc(0x90); break; // É
            case 0x00D1: putc(0xA5); break; // Ñ
            case 0x00D6: putc(0x99); break; // Ö
            case 0x00DC: putc(0x9A); break; // Ü
            case 0x00E1: putc(0xA0); break; // á
            case 0x00E0: putc(0x85); break; // à
            case 0x00E2: putc(0x83); break; // â
            case 0x00E4: putc(0x84); break; // ä
            case 0x00E3: putc(0x84); break; // ã (mapeado para ä se faltar em alguns modos, mas CP437 não tem ã)
            case 0x00E5: putc(0x86); break; // å
            case 0x00E7: putc(0x87); break; // ç
            case 0x00E9: putc(0x82); break; // é
            case 0x00E8: putc(0x8A); break; // è
            case 0x00EA: putc(0x88); break; // ê
            case 0x00EB: putc(0x89); break; // ë
            case 0x00ED: putc(0xA1); break; // í
            case 0x00EC: putc(0x8D); break; // ì
            case 0x00EE: putc(0x8C); break; // î
            case 0x00EF: putc(0x8B); break; // ï
            case 0x00F1: putc(0xA4); break; // ñ
            case 0x00F3: putc(0xA2); break; // ó
            case 0x00F2: putc(0x95); break; // ò
            case 0x00F4: putc(0x93); break; // ô
            case 0x00F6: putc(0x94); break; // ö
            case 0x00FA: putc(0xA3); break; // ú
            case 0x00F9: putc(0x97); break; // ù
            case 0x00FB: putc(0x96); break; // û
            case 0x00FC: putc(0x81); break; // ü
            case 0x00F5: putc(0x94); break; // õ (mapeado para ö/0x94 pois CP437 não tem õ)
            case 0x00C3: putc(0x8E); break; // Ã (mapeado para Ä/0x8E)
            case 0x00D5: putc(0x99); break; // Õ (mapeado para Ö/0x99)
            default:   putc('?');    break;
        }
    }
}

void putws(const uint16_t* str) {
    while (*str) {
        putwc(*str++);
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
                        putc_utf8(*fmt);
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
                        putc_utf8((char)*argp);
                        argp++;
                        break;

                    case 'C': // %C para caracteres UTF-16
                        putwc((uint16_t)*argp);
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

                    case 'S': // %S para strings UTF-16 (wide strings)
                        putws(*(const uint16_t**)argp);
                        argp++;
                        break;

                    case '%':   
                        putc_utf8('%');
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
        putc_utf8(buffer[pos]);

    return argp;
}
