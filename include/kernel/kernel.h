#ifndef KERNEL_H
#define KERNEL_H

#include <kernel/pic.h>
#include <kernel/irq.h>
#include <kernel/idt.h>
#include <kernel/keyboard.h>

void vga_clear(void);
void vga_putchar(char c);
void vga_print(const char *str);
void vga_print_hex(unsigned int n);
void vga_backspace(void);
int vga_get_cursor(void);

void idt_init(void);
void isr_handler(registers_t *regs);

#endif