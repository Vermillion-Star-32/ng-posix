#ifndef KERNEL_H
#define KERNEL_H

void vga_clear(void);
void vga_putchar(char c);
void vga_print(const char *str);
void vga_print_hex(unsigned int n);

#endif
