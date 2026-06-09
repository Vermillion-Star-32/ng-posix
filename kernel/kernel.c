#include "kernel.h"

void kernel_main(void) {
    vga_clear();
    vga_print("ng-posix\n");
    vga_print("----\n");
    vga_print("Running in 32-bit protected mode.\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
