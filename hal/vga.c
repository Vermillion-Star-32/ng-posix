#include <kernel/kernel.h>

#define VGA_BASE        ((volatile unsigned short *)0xB8000)
#define VGA_COLS        80
#define VGA_ROWS        25
#define WHITE_ON_BLACK  0x0F00

static int cursor = 0;

int vga_get_cursor(void) {
    return cursor;
}

void vga_clear(void) {
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++)
        VGA_BASE[i] = WHITE_ON_BLACK | ' ';
    cursor = 0;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor += VGA_COLS - (cursor % VGA_COLS);
        return;
    }
    VGA_BASE[cursor] = WHITE_ON_BLACK | (unsigned char)c;
    cursor++;
}

void vga_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++)
        vga_putchar(str[i]);
}

void vga_backspace(void) {
    if (cursor > 0 && (cursor % VGA_COLS) != 0) {
        cursor--;
        VGA_BASE[cursor] = WHITE_ON_BLACK | ' ';
    }
}

void vga_print_hex(unsigned int n) {
    char buf[10];
    int i = 0;
    vga_print("0x");
    if (n == 0) { vga_putchar('0'); return; }
    while (n > 0) {
        int d = n & 0xF;
        buf[i++] = d < 10 ? '0' + d : 'A' + d - 10;
        n >>= 4;
    }
    while (i-- > 0)
        vga_putchar(buf[i + 1]);
}
