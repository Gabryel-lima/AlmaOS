/**
 * @file test_keyboard.c
 * @brief Testa todos os mapeamentos de scancodes (normal / shift / AltGr)
 *        para os layouts US QWERTY e ABNT2.
 *
 * Compilação e execução no host:
 *   gcc -std=c11 -Wall -Wextra -o /tmp/test_keyboard \
 *       src/kernel/tests/test_keyboard.c && /tmp/test_keyboard
 *
 * As tabelas são cópias literais das definidas em keyboard.c; qualquer
 * divergência entre os dois arquivos constitui um erro de sincronização.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ── Framework mínimo de asserções ─────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

static void assert_eq_char(const char *label, char got, char expected) {
    if (got == expected) {
        g_pass++;
    } else {
        g_fail++;
        /* Imprime caracteres de controle como hex para legibilidade. */
        if ((unsigned char)got < 0x20 || (unsigned char)expected < 0x20)
            printf("  FAIL %-40s  got=0x%02X  expected=0x%02X\n",
                   label, (unsigned char)got, (unsigned char)expected);
        else
            printf("  FAIL %-40s  got='%c'(0x%02X)  expected='%c'(0x%02X)\n",
                   label, got, (unsigned char)got,
                   expected, (unsigned char)expected);
    }
}

/* ── Tabelas (cópias de keyboard.c) ─────────────────────────────────────────
 * ATENÇÃO: mantenha estas tabelas em sincronia com keyboard.c.
 * Se a tabela de produção mudar, atualizar aqui também.
 * ──────────────────────────────────────────────────────────────────────────── */

/* ── US QWERTY ─────────────────────────────────────────────────────────────── */

static const char sc_us[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   27,
    /* 0x02-0x0B: 1 2 3 4 5 6 7 8 9 0 */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    /* 0x0C (-), 0x0D (=), 0x0E (Backspace) */
    '-', '=', '\b',
    /* 0x0F (Tab), 0x10-0x19 (Q W E R T Y U I O P), 0x1A ([), 0x1B (]), 0x1C (Enter) */
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    /* 0x1D (Ctrl E), 0x1E-0x26 (A S D F G H J K L), 0x27 (;), 0x28 ('), 0x29 (`) */
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',
    /* 0x2A (Shift E), 0x2B (\), 0x2C-0x32 (Z X C V B N M), 0x33 (,), 0x34 (.), 0x35 (/), 0x36 (Shift D) */
    0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    /* 0x37 (* numpad), 0x38 (Alt E), 0x39 (Space), 0x3A (Caps Lock) */
    '*', 0,   ' ', 0,
    /* 0x3B-0x44: F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x45 (Num Lock), 0x46 (Scroll Lock) */
    0,   0,
    /* 0x47-0x53: numpad 7 8 9 - 4 5 6 + 1 2 3 0 Del */
    0,   0,   0,  '-',  0,   0,   0,  '+',  0,   0,   0,   0,   0,
    /* 0x54-0x56 */
    0,   0,   0,
    /* 0x57 (F11), 0x58 (F12) */
    0,   0,
    /* 0x59-0x7F: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char sc_us_shift[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   27,
    /* 0x02-0x0B: ! @ # $ % ^ & * ( ) */
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    /* 0x0C (_), 0x0D (+), 0x0E (Backspace) */
    '_', '+', '\b',
    /* 0x0F (Tab), 0x10-0x19 (Q W E R T Y U I O P), 0x1A ({), 0x1B (}), 0x1C (Enter) */
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    /* 0x1D (Ctrl), 0x1E-0x26 (A S D F G H J K L), 0x27 (:), 0x28 ("), 0x29 (~) */
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    /* 0x2A (Shift E), 0x2B (|), 0x2C-0x32 (Z X C V B N M), 0x33 (<), 0x34 (>), 0x35 (?), 0x36 (Shift D) */
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    /* 0x37 (* numpad), 0x38 (Alt), 0x39 (Space), 0x3A (Caps Lock) */
    '*', 0,   ' ', 0,
    /* 0x3B-0x44: F1-F10 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x45 (Num Lock), 0x46 (Scroll Lock) */
    0,   0,
    /* 0x47-0x53: numpad 7 8 9 - 4 5 6 + 1 2 3 0 Del */
    0,   0,   0,  '-',  0,   0,   0,  '+',  0,   0,   0,   0,   0,
    /* 0x54-0x56 */
    0,   0,   0,
    /* 0x57 (F11), 0x58 (F12) */
    0,   0,
    /* 0x59-0x7F: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char sc_us_altgr[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   0,
    /* 0x02-0x0B: 1 2 3 4 5 6 7 8 9 0 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x0C (-), 0x0D (=), 0x0E (Backspace) */
    0,   0,  '\b',
    /* 0x0F (Tab), 0x10 (Q→@), 0x11-0x19 (W E R T Y U I O P), 0x1A ([→{), 0x1B (]→}), 0x1C (Enter) */
    '\t','@', 0,   0,   0,   0,   0,   0,   0,   0,   0,  '{', '}', '\n',
    /* 0x1D (Ctrl E), 0x1E-0x26 (A S D F G H J K L), 0x27 (;), 0x28 ('), 0x29 (`) */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x2A (Shift E), 0x2B (\→|), 0x2C-0x30 (Z X C V B), 0x31 (N→~), 0x32-0x35 (M , . /) */
    0,  '|',  0,   0,   0,   0,   0,  '~',  0,   0,   0,   0,
    /* 0x36 (Shift D), 0x37 (* numpad), 0x38 (Alt E), 0x39 (Space), 0x3A (Caps Lock) */
    0,  '*',  0,  ' ',  0,
    /* 0x3B-0x44: F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x45 (Num Lock), 0x46 (Scroll Lock) */
    0,   0,
    /* 0x47-0x53: numpad 7 8 9 - 4 5 6 + 1 2 3 0 Del */
    0,   0,   0,  '-',  0,   0,   0,  '+',  0,   0,   0,   0,   0,
    /* 0x54-0x56 */
    0,   0,   0,
    /* 0x57 (F11), 0x58 (F12) */
    0,   0,
    /* 0x59-0x7F: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* ── ABNT2 ─────────────────────────────────────────────────────────────────── */

static const char sc_abnt2[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   27,
    /* 0x02-0x0B: 1 2 3 4 5 6 7 8 9 0 */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    /* 0x0C (-), 0x0D (=), 0x0E (Backspace) */
    '-', '=', '\b',
    /* 0x0F (Tab), 0x10-0x19 (Q W E R T Y U I O P), 0x1A (´→'), 0x1B ([), 0x1C (Enter) */
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\'','[', '\n',
    /* 0x1D (Ctrl E), 0x1E-0x26 (A S D F G H J K L), 0x27 (ç=0xE7), 0x28 (~), 0x29 (') */
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\xe7','~','\'',
    /* 0x2A (Shift E), 0x2B (]), 0x2C-0x32 (Z X C V B N M), 0x33 (,), 0x34 (.), 0x35 (;), 0x36 (Shift D) */
    0,   ']', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', ';', 0,
    /* 0x37 (* numpad), 0x38 (Alt E), 0x39 (Space), 0x3A (Caps Lock) */
    '*', 0,   ' ', 0,
    /* 0x3B-0x44: F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x45 (Num Lock), 0x46 (Scroll Lock) */
    0,   0,
    /* 0x47-0x53: numpad 7 8 9 - 4 5 6 + 1 2 3 0 Del */
    0,   0,   0,  '-',  0,   0,   0,  '+',  0,   0,   0,   0,   0,
    /* 0x54-0x56 */
    0,   0,   0,
    /* 0x57 (F11), 0x58 (F12) */
    0,   0,
    /* 0x59-0x72: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,
    /* 0x73: tecla extra ABNT2 (/) */
    '/',
    /* 0x74-0x7F: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char sc_abnt2_shift[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   27,
    /* 0x02-0x0B: ! @ # $ % [¨=0] & * ( ) */
    '!', '@', '#', '$', '%',  0,  '&', '*', '(', ')',
    /* 0x0C (_), 0x0D (+), 0x0E (Backspace) */
    '_', '+', '\b',
    /* 0x0F (Tab), 0x10-0x19 (Q W E R T Y U I O P), 0x1A (`), 0x1B ({), 0x1C (Enter) */
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', '\n',
    /* 0x1D (Ctrl), 0x1E-0x26 (A S D F G H J K L), 0x27 (Ç=0xC7), 0x28 (^), 0x29 (") */
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\xc7','^','"',
    /* 0x2A (Shift E), 0x2B (}), 0x2C-0x32 (Z X C V B N M), 0x33 (<), 0x34 (>), 0x35 (:), 0x36 (Shift D) */
    0,   '}', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', ':', 0,
    /* 0x37 (* numpad), 0x38 (Alt), 0x39 (Space), 0x3A (Caps Lock) */
    '*', 0,   ' ', 0,
    /* 0x3B-0x44: F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x45 (Num Lock), 0x46 (Scroll Lock) */
    0,   0,
    /* 0x47-0x53: numpad 7 8 9 - 4 5 6 + 1 2 3 0 Del */
    0,   0,   0,  '-',  0,   0,   0,  '+',  0,   0,   0,   0,   0,
    /* 0x54-0x56 */
    0,   0,   0,
    /* 0x57 (F11), 0x58 (F12) */
    0,   0,
    /* 0x59-0x72: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,
    /* 0x73: tecla extra ABNT2 Shift (?) */
    '?',
    /* 0x74-0x7F: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char sc_abnt2_altgr[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   0,
    /* 0x02-0x0B: 1 2 3 4 5 6 7 8 9 0 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x0C (-), 0x0D (=), 0x0E (Backspace) */
    0,   0,  '\b',
    /* 0x0F (Tab), 0x10 (Q), 0x11 (W), 0x12 (E[€=0]), 0x13-0x19 (R T Y U I O P), 0x1A (´→~), 0x1B ([→~), 0x1C (Enter) */
    '\t', 0,  0,   0,   0,   0,   0,   0,   0,   0,   0,  '~', '~','\n',
    /* 0x1D (Ctrl E), 0x1E-0x26 (A S D F G H J K L), 0x27, 0x28, 0x29 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x2A (Shift E), 0x2B (]→|), 0x2C-0x32 (Z X C V B N M), 0x33-0x35, 0x36 (Shift D) */
    0,  '|',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x36 (Shift D), 0x37 (* numpad), 0x38 (Alt E), 0x39 (Space), 0x3A (Caps Lock) */
    0,  '*',  0,  ' ',  0,
    /* 0x3B-0x44: F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x45 (Num Lock), 0x46 (Scroll Lock) */
    0,   0,
    /* 0x47-0x53: numpad 7 8 9 - 4 5 6 + 1 2 3 0 Del */
    0,   0,   0,  '-',  0,   0,   0,  '+',  0,   0,   0,   0,   0,
    /* 0x54-0x56 */
    0,   0,   0,
    /* 0x57 (F11), 0x58 (F12) */
    0,   0,
    /* 0x59-0x72: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,
    /* 0x73: tecla extra ABNT2 AltGr */
    0,
    /* 0x74-0x7F: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* ── Testes por grupo ─────────────────────────────────────────────────────── */

static void test_us_normal(void) {
    printf("[US normal]\n");
    assert_eq_char("0x01 Esc",          sc_us[0x01], 27);
    assert_eq_char("0x02 '1'",          sc_us[0x02], '1');
    assert_eq_char("0x03 '2'",          sc_us[0x03], '2');
    assert_eq_char("0x04 '3'",          sc_us[0x04], '3');
    assert_eq_char("0x05 '4'",          sc_us[0x05], '4');
    assert_eq_char("0x06 '5'",          sc_us[0x06], '5');
    assert_eq_char("0x07 '6'",          sc_us[0x07], '6');
    assert_eq_char("0x08 '7'",          sc_us[0x08], '7');
    assert_eq_char("0x09 '8'",          sc_us[0x09], '8');
    assert_eq_char("0x0A '9'",          sc_us[0x0A], '9');
    assert_eq_char("0x0B '0'",          sc_us[0x0B], '0');
    assert_eq_char("0x0C '-'",          sc_us[0x0C], '-');
    assert_eq_char("0x0D '='",          sc_us[0x0D], '=');
    assert_eq_char("0x0E Backspace",    sc_us[0x0E], '\b');
    assert_eq_char("0x0F Tab",          sc_us[0x0F], '\t');
    assert_eq_char("0x10 'q'",          sc_us[0x10], 'q');
    assert_eq_char("0x11 'w'",          sc_us[0x11], 'w');
    assert_eq_char("0x12 'e'",          sc_us[0x12], 'e');
    assert_eq_char("0x13 'r'",          sc_us[0x13], 'r');
    assert_eq_char("0x14 't'",          sc_us[0x14], 't');
    assert_eq_char("0x15 'y'",          sc_us[0x15], 'y');
    assert_eq_char("0x16 'u'",          sc_us[0x16], 'u');
    assert_eq_char("0x17 'i'",          sc_us[0x17], 'i');
    assert_eq_char("0x18 'o'",          sc_us[0x18], 'o');
    assert_eq_char("0x19 'p'",          sc_us[0x19], 'p');
    assert_eq_char("0x1A '['",          sc_us[0x1A], '[');
    assert_eq_char("0x1B ']'",          sc_us[0x1B], ']');
    assert_eq_char("0x1C Enter",        sc_us[0x1C], '\n');
    assert_eq_char("0x1D Ctrl (null)",  sc_us[0x1D], 0);
    assert_eq_char("0x1E 'a'",          sc_us[0x1E], 'a');
    assert_eq_char("0x1F 's'",          sc_us[0x1F], 's');
    assert_eq_char("0x20 'd'",          sc_us[0x20], 'd');
    assert_eq_char("0x21 'f'",          sc_us[0x21], 'f');
    assert_eq_char("0x22 'g'",          sc_us[0x22], 'g');
    assert_eq_char("0x23 'h'",          sc_us[0x23], 'h');
    assert_eq_char("0x24 'j'",          sc_us[0x24], 'j');
    assert_eq_char("0x25 'k'",          sc_us[0x25], 'k');
    assert_eq_char("0x26 'l'",          sc_us[0x26], 'l');
    assert_eq_char("0x27 ';'",          sc_us[0x27], ';');
    assert_eq_char("0x28 apostrofo",    sc_us[0x28], '\'');
    assert_eq_char("0x29 grave '`'",    sc_us[0x29], '`');
    assert_eq_char("0x2A ShiftL (null)",sc_us[0x2A], 0);
    assert_eq_char("0x2B '\\'",         sc_us[0x2B], '\\');
    assert_eq_char("0x2C 'z'",          sc_us[0x2C], 'z');
    assert_eq_char("0x2D 'x'",          sc_us[0x2D], 'x');
    assert_eq_char("0x2E 'c'",          sc_us[0x2E], 'c');
    assert_eq_char("0x2F 'v'",          sc_us[0x2F], 'v');
    assert_eq_char("0x30 'b'",          sc_us[0x30], 'b');
    assert_eq_char("0x31 'n'",          sc_us[0x31], 'n');
    assert_eq_char("0x32 'm'",          sc_us[0x32], 'm');
    assert_eq_char("0x33 ','",          sc_us[0x33], ',');
    assert_eq_char("0x34 '.'",          sc_us[0x34], '.');
    assert_eq_char("0x35 '/'",          sc_us[0x35], '/');
    assert_eq_char("0x36 ShiftR (null)",sc_us[0x36], 0);
    assert_eq_char("0x37 '* numpad'",   sc_us[0x37], '*');
    assert_eq_char("0x38 AltL (null)",  sc_us[0x38], 0);
    assert_eq_char("0x39 Space",        sc_us[0x39], ' ');
    /* F keys e Num Lock devem ser 0 */
    for (int i = 0x3B; i <= 0x44; i++)
        assert_eq_char("0x3B-0x44 F1-F10 (null)", sc_us[i], 0);
    assert_eq_char("0x45 NumLock (null)",  sc_us[0x45], 0);
    assert_eq_char("0x46 ScrollLock (null)",sc_us[0x46], 0);
    assert_eq_char("0x4A numpad '-'",   sc_us[0x4A], '-');
    assert_eq_char("0x4E numpad '+'",   sc_us[0x4E], '+');
    assert_eq_char("0x57 F11 (null)",   sc_us[0x57], 0);
    assert_eq_char("0x58 F12 (null)",   sc_us[0x58], 0);
}

static void test_us_shift(void) {
    printf("[US shift]\n");
    assert_eq_char("0x02 '!'",   sc_us_shift[0x02], '!');
    assert_eq_char("0x03 '@'",   sc_us_shift[0x03], '@');
    assert_eq_char("0x04 '#'",   sc_us_shift[0x04], '#');
    assert_eq_char("0x05 '$'",   sc_us_shift[0x05], '$');
    assert_eq_char("0x06 '%'",   sc_us_shift[0x06], '%');
    assert_eq_char("0x07 '^'",   sc_us_shift[0x07], '^');
    assert_eq_char("0x08 '&'",   sc_us_shift[0x08], '&');
    assert_eq_char("0x09 '*'",   sc_us_shift[0x09], '*');
    assert_eq_char("0x0A '('",   sc_us_shift[0x0A], '(');
    assert_eq_char("0x0B ')'",   sc_us_shift[0x0B], ')');
    assert_eq_char("0x0C '_'",   sc_us_shift[0x0C], '_');
    assert_eq_char("0x0D '+'",   sc_us_shift[0x0D], '+');
    assert_eq_char("0x10 'Q'",   sc_us_shift[0x10], 'Q');
    assert_eq_char("0x11 'W'",   sc_us_shift[0x11], 'W');
    assert_eq_char("0x12 'E'",   sc_us_shift[0x12], 'E');
    assert_eq_char("0x13 'R'",   sc_us_shift[0x13], 'R');
    assert_eq_char("0x14 'T'",   sc_us_shift[0x14], 'T');
    assert_eq_char("0x15 'Y'",   sc_us_shift[0x15], 'Y');
    assert_eq_char("0x16 'U'",   sc_us_shift[0x16], 'U');
    assert_eq_char("0x17 'I'",   sc_us_shift[0x17], 'I');
    assert_eq_char("0x18 'O'",   sc_us_shift[0x18], 'O');
    assert_eq_char("0x19 'P'",   sc_us_shift[0x19], 'P');
    assert_eq_char("0x1A '{'",   sc_us_shift[0x1A], '{');
    assert_eq_char("0x1B '}'",   sc_us_shift[0x1B], '}');
    assert_eq_char("0x1E 'A'",   sc_us_shift[0x1E], 'A');
    assert_eq_char("0x1F 'S'",   sc_us_shift[0x1F], 'S');
    assert_eq_char("0x20 'D'",   sc_us_shift[0x20], 'D');
    assert_eq_char("0x21 'F'",   sc_us_shift[0x21], 'F');
    assert_eq_char("0x22 'G'",   sc_us_shift[0x22], 'G');
    assert_eq_char("0x23 'H'",   sc_us_shift[0x23], 'H');
    assert_eq_char("0x24 'J'",   sc_us_shift[0x24], 'J');
    assert_eq_char("0x25 'K'",   sc_us_shift[0x25], 'K');
    assert_eq_char("0x26 'L'",   sc_us_shift[0x26], 'L');
    assert_eq_char("0x27 ':'",   sc_us_shift[0x27], ':');
    assert_eq_char("0x28 '\"'",  sc_us_shift[0x28], '"');
    assert_eq_char("0x29 '~'",   sc_us_shift[0x29], '~');
    assert_eq_char("0x2B '|'",   sc_us_shift[0x2B], '|');
    assert_eq_char("0x2C 'Z'",   sc_us_shift[0x2C], 'Z');
    assert_eq_char("0x2D 'X'",   sc_us_shift[0x2D], 'X');
    assert_eq_char("0x2E 'C'",   sc_us_shift[0x2E], 'C');
    assert_eq_char("0x2F 'V'",   sc_us_shift[0x2F], 'V');
    assert_eq_char("0x30 'B'",   sc_us_shift[0x30], 'B');
    assert_eq_char("0x31 'N'",   sc_us_shift[0x31], 'N');
    assert_eq_char("0x32 'M'",   sc_us_shift[0x32], 'M');
    assert_eq_char("0x33 '<'",   sc_us_shift[0x33], '<');
    assert_eq_char("0x34 '>'",   sc_us_shift[0x34], '>');
    assert_eq_char("0x35 '?'",   sc_us_shift[0x35], '?');
    /* Modificadores devem ser 0 mesmo com shift */
    assert_eq_char("0x2A ShiftL (null)", sc_us_shift[0x2A], 0);
    assert_eq_char("0x36 ShiftR (null)", sc_us_shift[0x36], 0);
    assert_eq_char("0x1D Ctrl (null)",   sc_us_shift[0x1D], 0);
}

static void test_us_altgr(void) {
    printf("[US AltGr]\n");
    assert_eq_char("0x10 Q→'@'",  sc_us_altgr[0x10], '@');
    assert_eq_char("0x1A [→'{'",  sc_us_altgr[0x1A], '{');
    assert_eq_char("0x1B ]→'}'",  sc_us_altgr[0x1B], '}');
    assert_eq_char("0x2B \\→'|'", sc_us_altgr[0x2B], '|');
    assert_eq_char("0x31 N→'~'",  sc_us_altgr[0x31], '~');
    assert_eq_char("0x39 Space",   sc_us_altgr[0x39], ' ');
    assert_eq_char("0x0E Backspace", sc_us_altgr[0x0E], '\b');
    /* Letras sem AltGr US devem ser 0 */
    assert_eq_char("0x11 W (null)", sc_us_altgr[0x11], 0);
    assert_eq_char("0x12 E (null)", sc_us_altgr[0x12], 0);
    assert_eq_char("0x1E A (null)", sc_us_altgr[0x1E], 0);
}

static void test_abnt2_normal(void) {
    printf("[ABNT2 normal]\n");
    assert_eq_char("0x01 Esc",           sc_abnt2[0x01], 27);
    assert_eq_char("0x02 '1'",           sc_abnt2[0x02], '1');
    assert_eq_char("0x0C '-'",           sc_abnt2[0x0C], '-');
    assert_eq_char("0x0D '='",           sc_abnt2[0x0D], '=');
    assert_eq_char("0x0E Backspace",     sc_abnt2[0x0E], '\b');
    assert_eq_char("0x0F Tab",           sc_abnt2[0x0F], '\t');
    assert_eq_char("0x10 'q'",           sc_abnt2[0x10], 'q');
    assert_eq_char("0x1A dead acute→'",  sc_abnt2[0x1A], '\'');
    assert_eq_char("0x1B '['",           sc_abnt2[0x1B], '[');
    assert_eq_char("0x1C Enter",         sc_abnt2[0x1C], '\n');
    assert_eq_char("0x27 ç (0xE7)",      (unsigned char)sc_abnt2[0x27], 0xE7);
    assert_eq_char("0x28 dead tilde~",   sc_abnt2[0x28], '~');
    assert_eq_char("0x29 apostrofo",     sc_abnt2[0x29], '\'');
    assert_eq_char("0x2B ']'",           sc_abnt2[0x2B], ']');
    assert_eq_char("0x35 ';'",           sc_abnt2[0x35], ';');
    assert_eq_char("0x39 Space",         sc_abnt2[0x39], ' ');
    assert_eq_char("0x73 '/' extra",     sc_abnt2[0x73], '/');
    /* Modificadores devem ser 0 */
    assert_eq_char("0x2A ShiftL (null)", sc_abnt2[0x2A], 0);
    assert_eq_char("0x36 ShiftR (null)", sc_abnt2[0x36], 0);
    assert_eq_char("0x1D Ctrl (null)",   sc_abnt2[0x1D], 0);
    assert_eq_char("0x38 Alt (null)",    sc_abnt2[0x38], 0);
}

static void test_abnt2_shift(void) {
    printf("[ABNT2 shift]\n");
    assert_eq_char("0x02 '!'",         sc_abnt2_shift[0x02], '!');
    assert_eq_char("0x03 '@'",         sc_abnt2_shift[0x03], '@');
    assert_eq_char("0x04 '#'",         sc_abnt2_shift[0x04], '#');
    assert_eq_char("0x05 '$'",         sc_abnt2_shift[0x05], '$');
    assert_eq_char("0x06 '%'",         sc_abnt2_shift[0x06], '%');
    assert_eq_char("0x07 diarese=0",   sc_abnt2_shift[0x07], 0);
    assert_eq_char("0x08 '&'",         sc_abnt2_shift[0x08], '&');
    assert_eq_char("0x09 '*'",         sc_abnt2_shift[0x09], '*');
    assert_eq_char("0x0A '('",         sc_abnt2_shift[0x0A], '(');
    assert_eq_char("0x0B ')'",         sc_abnt2_shift[0x0B], ')');
    assert_eq_char("0x0C '_'",         sc_abnt2_shift[0x0C], '_');
    assert_eq_char("0x0D '+'",         sc_abnt2_shift[0x0D], '+');
    assert_eq_char("0x10 'Q'",         sc_abnt2_shift[0x10], 'Q');
    assert_eq_char("0x1A dead grave`", sc_abnt2_shift[0x1A], '`');
    assert_eq_char("0x1B '{'",         sc_abnt2_shift[0x1B], '{');
    assert_eq_char("0x27 Ç (0xC7)",    (unsigned char)sc_abnt2_shift[0x27], 0xC7);
    assert_eq_char("0x28 dead circ^",  sc_abnt2_shift[0x28], '^');
    assert_eq_char("0x29 '\"'",        sc_abnt2_shift[0x29], '"');
    assert_eq_char("0x2B '}'",         sc_abnt2_shift[0x2B], '}');
    assert_eq_char("0x35 ':'",         sc_abnt2_shift[0x35], ':');
    assert_eq_char("0x39 Space",       sc_abnt2_shift[0x39], ' ');
    assert_eq_char("0x73 '?' extra",   sc_abnt2_shift[0x73], '?');
}

static void test_abnt2_altgr(void) {
    printf("[ABNT2 AltGr]\n");
    assert_eq_char("0x0E Backspace",   sc_abnt2_altgr[0x0E], '\b');
    assert_eq_char("0x0F Tab",         sc_abnt2_altgr[0x0F], '\t');
    assert_eq_char("0x1A ´→'~'",       sc_abnt2_altgr[0x1A], '~');
    assert_eq_char("0x1B [→'~'",       sc_abnt2_altgr[0x1B], '~');
    assert_eq_char("0x1C Enter",       sc_abnt2_altgr[0x1C], '\n');
    assert_eq_char("0x2B ]→'|'",       sc_abnt2_altgr[0x2B], '|');
    assert_eq_char("0x39 Space",       sc_abnt2_altgr[0x39], ' ');
    assert_eq_char("0x73 extra=0",     sc_abnt2_altgr[0x73], 0);
    /* €, ², ³ não mapeáveis em char → 0 */
    assert_eq_char("0x02 1[²=0]",     sc_abnt2_altgr[0x02], 0);
    assert_eq_char("0x04 3[³=0]",     sc_abnt2_altgr[0x04], 0);
    assert_eq_char("0x12 E[€=0]",     sc_abnt2_altgr[0x12], 0);
}

static void test_symmetry_us(void) {
    printf("[US simetria normal/shift letras]\n");
    /* Para cada letra minúscula, a versão shift deve ser maiúscula. */
    static const struct { uint8_t sc; char lower; char upper; } letters[] = {
        {0x10,'q','Q'},{0x11,'w','W'},{0x12,'e','E'},{0x13,'r','R'},
        {0x14,'t','T'},{0x15,'y','Y'},{0x16,'u','U'},{0x17,'i','I'},
        {0x18,'o','O'},{0x19,'p','P'},{0x1E,'a','A'},{0x1F,'s','S'},
        {0x20,'d','D'},{0x21,'f','F'},{0x22,'g','G'},{0x23,'h','H'},
        {0x24,'j','J'},{0x25,'k','K'},{0x26,'l','L'},{0x2C,'z','Z'},
        {0x2D,'x','X'},{0x2E,'c','C'},{0x2F,'v','V'},{0x30,'b','B'},
        {0x31,'n','N'},{0x32,'m','M'},
    };
    char label[64];
    for (size_t i = 0; i < sizeof(letters)/sizeof(letters[0]); i++) {
        snprintf(label, sizeof(label), "0x%02X normal='%c'", letters[i].sc, letters[i].lower);
        assert_eq_char(label, sc_us[letters[i].sc], letters[i].lower);
        snprintf(label, sizeof(label), "0x%02X shift='%c'",  letters[i].sc, letters[i].upper);
        assert_eq_char(label, sc_us_shift[letters[i].sc], letters[i].upper);
    }
}

static void test_symmetry_abnt2(void) {
    printf("[ABNT2 simetria normal/shift letras]\n");
    static const struct { uint8_t sc; char lower; char upper; } letters[] = {
        {0x10,'q','Q'},{0x11,'w','W'},{0x12,'e','E'},{0x13,'r','R'},
        {0x14,'t','T'},{0x15,'y','Y'},{0x16,'u','U'},{0x17,'i','I'},
        {0x18,'o','O'},{0x19,'p','P'},{0x1E,'a','A'},{0x1F,'s','S'},
        {0x20,'d','D'},{0x21,'f','F'},{0x22,'g','G'},{0x23,'h','H'},
        {0x24,'j','J'},{0x25,'k','K'},{0x26,'l','L'},{0x2C,'z','Z'},
        {0x2D,'x','X'},{0x2E,'c','C'},{0x2F,'v','V'},{0x30,'b','B'},
        {0x31,'n','N'},{0x32,'m','M'},
    };
    char label[64];
    for (size_t i = 0; i < sizeof(letters)/sizeof(letters[0]); i++) {
        snprintf(label, sizeof(label), "0x%02X normal='%c'", letters[i].sc, letters[i].lower);
        assert_eq_char(label, sc_abnt2[letters[i].sc], letters[i].lower);
        snprintf(label, sizeof(label), "0x%02X shift='%c'",  letters[i].sc, letters[i].upper);
        assert_eq_char(label, sc_abnt2_shift[letters[i].sc], letters[i].upper);
    }
}

static void test_table_size(void) {
    printf("[Tamanho das tabelas = 128]\n");
    /* Verificação em tempo de compilação via sizeof. */
    assert_eq_char("sizeof sc_us == 128",          (char)(sizeof(sc_us)          == 128), 1);
    assert_eq_char("sizeof sc_us_shift == 128",    (char)(sizeof(sc_us_shift)    == 128), 1);
    assert_eq_char("sizeof sc_us_altgr == 128",    (char)(sizeof(sc_us_altgr)    == 128), 1);
    assert_eq_char("sizeof sc_abnt2 == 128",       (char)(sizeof(sc_abnt2)       == 128), 1);
    assert_eq_char("sizeof sc_abnt2_shift == 128", (char)(sizeof(sc_abnt2_shift) == 128), 1);
    assert_eq_char("sizeof sc_abnt2_altgr == 128", (char)(sizeof(sc_abnt2_altgr) == 128), 1);
}

/* ── main ─────────────────────────────────────────────────────────────────── */

int main(void) {
    test_table_size();
    test_us_normal();
    test_us_shift();
    test_us_altgr();
    test_abnt2_normal();
    test_abnt2_shift();
    test_abnt2_altgr();
    test_symmetry_us();
    test_symmetry_abnt2();

    printf("\nResultado: %d passaram, %d falharam.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
