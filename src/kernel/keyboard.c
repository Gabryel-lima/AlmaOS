#include "include/keyboard.h"
#include "include/io.h"

/* Buffer circular */
static volatile char    kb_buffer[KB_BUFFER_SIZE];
static volatile uint8_t kb_head;
static volatile uint8_t kb_tail;

/* Estado de shift, AltGr e Caps Lock */
static volatile bool kb_shift;  /* Shift (Left/Right) */
static volatile bool kb_altgr;  /* AltGr (Right Alt) */
static volatile bool kb_caps;   /* Caps Lock (toggle) */
static volatile bool kb_ext;    /* prefixo 0xE0 pendente */

/* ── Tabelas US QWERTY ─────────────────────────────────────────────────────── */

/* Scancode set 1 → ASCII (US QWERTY, normal) */
static const char scancode_to_ascii_us[128] = {
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

/* Scancode set 1 → ASCII (US QWERTY, shifted) */
static const char scancode_to_ascii_shift_us[128] = {
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

/* Scancode set 1 → ASCII (US QWERTY, AltGr)
 * No teclado físico US não há tecla AltGr nativa; Right Alt é mapeado
 * a este layer para conveniência (Q→@, [→{, ]→}, \→|, N→~). */
static const char scancode_to_ascii_altgr_us[128] = {
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

/* ── Tabelas ABNT2 ─────────────────────────────────────────────────────────── */

/* Scancode set 1 → ASCII (ABNT2, normal)
 * Dead keys (0x1A ´ acute, 0x28 ~ tilde) mapeados diretamente ao caractere base;
 * combinação de acento não é suportada neste driver.
 * ç/Ç usam bytes ISO-8859-1 (0xE7/0xC7); o VGA deve renderizar Latin-1 ou CP850. */
static const char scancode_to_ascii_abnt2[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   27,
    /* 0x02-0x0B: 1 2 3 4 5 6 7 8 9 0 */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    /* 0x0C (-), 0x0D (=), 0x0E (Backspace) */
    '-', '=', '\b',
    /* 0x0F (Tab), 0x10-0x19 (Q W E R T Y U I O P), 0x1A (´ dead acute→'), 0x1B ([), 0x1C (Enter) */
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\'','[', '\n',
    /* 0x1D (Ctrl E), 0x1E-0x26 (A S D F G H J K L), 0x27 (ç), 0x28 (~ dead tilde), 0x29 (') */
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
    /* 0x73: tecla extra ABNT2 (/ entre . e Shift D) */
    '/',
    /* 0x74-0x7F: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* Scancode set 1 → ASCII (ABNT2, shifted)
 * Shift+6 = ¨ (diarese, U+00A8) não representável em char; mapeado como 0.
 * Ç usa byte ISO-8859-1 0xC7.
 * Dead keys (0x1A → ` grave, 0x28 → ^ circunflexo) mapeados diretamente. */
static const char scancode_to_ascii_shift_abnt2[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   27,
    /* 0x02-0x0B: ! @ # $ % [¨=N/A] & * ( ) */
    '!', '@', '#', '$', '%',  0,  '&', '*', '(', ')',
    /* 0x0C (_), 0x0D (+), 0x0E (Backspace) */
    '_', '+', '\b',
    /* 0x0F (Tab), 0x10-0x19 (Q W E R T Y U I O P), 0x1A (` dead grave), 0x1B ({), 0x1C (Enter) */
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', '\n',
    /* 0x1D (Ctrl), 0x1E-0x26 (A S D F G H J K L), 0x27 (Ç), 0x28 (^ dead circ), 0x29 (") */
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

/* Scancode set 1 → ASCII (ABNT2, AltGr)
 * A maioria dos caracteres AltGr do ABNT2 (², ³, €) são multi-byte UTF-8
 * e não representáveis em char. Apenas mapeamentos de byte único são incluídos. */
static const char scancode_to_ascii_altgr_abnt2[128] = {
    /* 0x00 (null), 0x01 (Esc) */
    0,   0,
    /* 0x02-0x0B: 1[²=N/A] 2 3[³=N/A] 4 5 6 7 8 9 0 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x0C (-), 0x0D (=), 0x0E (Backspace) */
    0,   0,  '\b',
    /* 0x0F (Tab), 0x10-0x11 (Q W), 0x12 (E[€=N/A]), 0x13-0x19 (R T Y U I O P), 0x1A (´→~), 0x1B ([→~), 0x1C (Enter) */
    '\t', 0,  0,   0,   0,   0,   0,   0,   0,   0,   0,  '~', '~','\n',
    /* 0x1D (Ctrl E), 0x1E-0x26 (A S D F G H J K L), 0x27 (ç), 0x28 (~), 0x29 (') */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    /* 0x2A (Shift E), 0x2B (]→|), 0x2C-0x32 (Z X C V B N M), 0x33 (,), 0x34 (.), 0x35 (;) */
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
    /* 0x73: tecla extra ABNT2 AltGr (sem mapeamento padrão) */
    0,
    /* 0x74-0x7F: scancodes não utilizados */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* ── Ponteiros ativos (selecionados pelo layout) ───────────────────────────── */
static const char *kb_map       = scancode_to_ascii_abnt2;
static const char *kb_map_shift = scancode_to_ascii_shift_abnt2;
static const char *kb_map_altgr = scancode_to_ascii_altgr_abnt2;

static void kb_push(char c) {
    uint8_t next = (kb_head + 1) % KB_BUFFER_SIZE;
    if (next != kb_tail) {
        kb_buffer[kb_head] = c;
        kb_head = next;
    }
}

void keyboard_init(void) {
    kb_head   = 0;
    kb_tail   = 0;
    kb_shift  = false;
    kb_altgr  = false;
    kb_caps   = false;
    kb_ext    = false;
    keyboard_set_layout(KB_LAYOUT_ABNT2);
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KB_DATA_PORT);

    /* prefixo de scancode estendido */
    if (scancode == 0xE0) {
        kb_ext = true;
        return;
    }

    /* break code (key release) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36)   /* left/right shift */
            kb_shift = false;
        if (kb_ext && released == 0x38)             /* Right Alt (AltGr) release */
            kb_altgr = false;
        kb_ext = false;
        return;
    }

    /* make code (key press) */
    if (scancode == 0x2A || scancode == 0x36) {
        kb_shift = true;
        kb_ext = false;
        return;
    }
    if (scancode == 0x3A) {                         /* Caps Lock toggle */
        kb_caps = !kb_caps;
        kb_ext = false;
        return;
    }
    if (kb_ext && scancode == 0x38) {               /* Right Alt (AltGr) press */
        kb_altgr = true;
        kb_ext = false;
        return;
    }
    kb_ext = false;

    char c;
    if (kb_altgr)
        c = kb_map_altgr[scancode];
    else if (kb_shift)
        c = kb_map_shift[scancode];
    else
        c = kb_map[scancode];

    /* Caps Lock: inverte caixa apenas para letras */
    if (kb_caps && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
        c ^= 0x20;

    if (c)
        kb_push(c);
}

char keyboard_read(void) {
    if (kb_head == kb_tail) return 0;
    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

char keyboard_getchar(void) {
    while (kb_head == kb_tail)
        __asm__ volatile("hlt");
    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

bool keyboard_has_data(void) {
    return kb_head != kb_tail;
}

void keyboard_set_layout(kb_layout_t layout) {
    if (layout == KB_LAYOUT_ABNT2) {
        kb_map       = scancode_to_ascii_abnt2;
        kb_map_shift = scancode_to_ascii_shift_abnt2;
        kb_map_altgr = scancode_to_ascii_altgr_abnt2;
    } else {
        kb_map       = scancode_to_ascii_us;
        kb_map_shift = scancode_to_ascii_shift_us;
        kb_map_altgr = scancode_to_ascii_altgr_us;
    }
}

kb_layout_t keyboard_get_layout(void) {
    return (kb_map == scancode_to_ascii_abnt2) ? KB_LAYOUT_ABNT2 : KB_LAYOUT_US;
}
