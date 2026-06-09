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

    vga_print("Running in 32-bit protected mode.\n");

    __asm__ volatile ("sti");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}