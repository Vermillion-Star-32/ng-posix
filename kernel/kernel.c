#include "kernel.h"

void kernel_main(void) {
    vga_clear();
    vga_print("ng-posix\n");
    vga_print("--------\n");

    vga_print("Initialising IDT...\n");
    idt_init();
    vga_print("IDT OK\n");

    vga_print("Remapping PIC...\n");
    pic_remap(PIC1_OFFSET, PIC2_OFFSET);
    pic_disable();
    vga_print("PIC OK\n");

    vga_print("Initialising IRQs...\n");
    irq_init();
    vga_print("IRQ OK\n");

    vga_print("Initialising keyboard...\n");
    keyboard_init();
    vga_print("Keyboard OK\n");

    vga_print("Ready. Type below:\n> ");

    __asm__ volatile ("sti");

    for (;;) {
        char c = keyboard_getchar();
        if (c) {
            if (c == '\r' || c == '\n') {
                vga_print("\n> ");
            } else {
                vga_putchar(c);
            }
        }
        __asm__ volatile ("hlt");
    }
}
