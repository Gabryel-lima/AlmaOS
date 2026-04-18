#include "include/keyboard.h"
#include "include/io.h"

/* Buffer circular */
static volatile char    kb_buffer[KB_BUFFER_SIZE];
static volatile uint8_t kb_head;
static volatile uint8_t kb_tail;

/* Estado de shift */
static volatile bool kb_shift;

/* Scancode set 1 → ASCII (US QWERTY, lowercase) */
static const char scancode_to_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,  ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* F1-F10 */
    0, 0,                               /* Num Lock, Scroll Lock */
    0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0,
    0, 0, 0,
    0, 0,                               /* F11, F12 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Scancode set 1 → ASCII (shifted) */
static const char scancode_to_ascii_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,  'A','S','D','F','G','H','J','K','L',':','"','~',
    0,  '|','Z','X','C','V','B','N','M','<','>','?',0,
    '*',0,  ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,
    0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0,
    0, 0, 0,
    0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0
};

static void kb_push(char c) {
    uint8_t next = (kb_head + 1) % KB_BUFFER_SIZE;
    if (next != kb_tail) {
        kb_buffer[kb_head] = c;
        kb_head = next;
    }
}

void keyboard_init(void) {
    kb_head  = 0;
    kb_tail  = 0;
    kb_shift = false;
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KB_DATA_PORT);

    /* break code (key release) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36)   /* left/right shift */
            kb_shift = false;
        return;
    }

    /* make code (key press) */
    if (scancode == 0x2A || scancode == 0x36) {
        kb_shift = true;
        return;
    }

    char c;
    if (kb_shift)
        c = scancode_to_ascii_shift[scancode];
    else
        c = scancode_to_ascii[scancode];

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
