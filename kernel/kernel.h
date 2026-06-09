#ifndef KERNEL_H
#define KERNEL_H

void vga_clear(void);
void vga_putchar(char c);
void vga_print(const char *str);
void vga_print_hex(unsigned int n);

void idt_init(void);

#include "idt.h"
void isr_handler(registers_t *regs);

#include "pic.h"
void pic_remap(unsigned char offset1, unsigned char offset2);
void pic_mask(unsigned char irq);
void pic_unmask(unsigned char irq);
void pic_eoi(unsigned char irq);
void pic_disable(void);

#endif