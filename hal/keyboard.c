#include <kernel/irq.h>
#include <kernel/pic.h>
#include <kernel/keyboard.h>

#define KB_DATA_PORT  0x60
#define KB_BUF_SIZE   256

static const char scancode_table[128] = {
      0,   27, '1', '2', '3', '4', '5', '6', '7', '8',  /* 0x00-0x09 */
    '9',  '0', '-', '=',   8,   9, 'q', 'w', 'e', 'r',  /* 0x0A-0x13 */
    't',  'y', 'u', 'i', 'o', 'p', '[', ']',  13,   0,  /* 0x14-0x1D */
    'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',  /* 0x1E-0x27 */
    '\'', '`',   0, '\\','z', 'x', 'c', 'v', 'b', 'n',  /* 0x28-0x31 */
    'm',  ',', '.', '/',   0, '*',   0, ' ',   0,   0,   /* 0x32-0x3B */
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x3C-0x45 */
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x46-0x4F */
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x50-0x59 */
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x5A-0x63 */
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x64-0x6D */
      0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x6E-0x77 */
      0,    0,   0,   0,   0,   0,   0,   0              /* 0x78-0x7F */
};

static char     kb_buf[KB_BUF_SIZE];
static int      kb_head = 0;
static int      kb_tail = 0;

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
    unsigned char scancode = inb(KB_DATA_PORT);

    if (scancode & 0x80) return;

    char c = scancode_table[scancode];
    if (c) kb_buf_push(c);
}

void keyboard_init(void) {
    irq_register(1, keyboard_handler);
    pic_unmask(IRQ_KEYBOARD);
}
