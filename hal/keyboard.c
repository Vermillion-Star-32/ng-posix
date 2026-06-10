#include <kernel/irq.h>
#include <kernel/pic.h>
#include <kernel/keyboard.h>
#include <kernel/kernel.h>

#define KB_DATA_PORT  0x60
#define KB_BUF_SIZE   256

static const char scancode_table[128] = {
      0,   27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9',  '0', '-', '=',   8,   9, 'q', 'w', 'e', 'r',
    't',  'y', 'u', 'i', 'o', 'p', '[', ']',  13,   0,
    'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`',   0, '\\','z', 'x', 'c', 'v', 'b', 'n',
    'm',  ',', '.', '/',   0, '*',   0, ' ',   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0
};

static const char scancode_table_shift[128] = {
      0,   27, '!', '@', '#', '$', '%', '^', '&', '*',
    '(',  ')', '_', '+',   8,   9, 'Q', 'W', 'E', 'R',
    'T',  'Y', 'U', 'I', 'O', 'P', '{', '}',  13,   0,
    'A',  'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"',  '~',   0, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M',  '<', '>', '?',   0, '*',   0, ' ',   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0,   0
};

#define SC_LSHIFT     0x2A
#define SC_RSHIFT     0x36
#define SC_LSHIFT_REL 0xAA
#define SC_RSHIFT_REL 0xB6
#define SC_BACKSPACE  0x0E

static char kb_buf[KB_BUF_SIZE];
static int  kb_head = 0;
static int  kb_tail = 0;
static int  shift_held = 0;

static void kb_buf_push(char c) {
    int next = (kb_head + 1) % KB_BUF_SIZE;
    if (next != kb_tail) {
        kb_buf[kb_head] = c;
        kb_head = next;
    }
}

char keyboard_getchar(void) {
    if (kb_tail == kb_head) return 0;
    char c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUF_SIZE;
    return c;
}

static void keyboard_handler(registers_t *regs) {
    (void)regs;
    unsigned char sc = inb(KB_DATA_PORT);

    if (sc == SC_LSHIFT_REL || sc == SC_RSHIFT_REL) { shift_held = 0; return; }
    if (sc & 0x80) return;
    if (sc == SC_LSHIFT || sc == SC_RSHIFT) { shift_held = 1; return; }
    if (sc == SC_BACKSPACE) { kb_buf_push('\b'); return; }

    char c = shift_held ? scancode_table_shift[sc] : scancode_table[sc];
    if (c) kb_buf_push(c);
}

void keyboard_init(void) {
    irq_register(1, keyboard_handler);
    pic_unmask(IRQ_KEYBOARD);
}