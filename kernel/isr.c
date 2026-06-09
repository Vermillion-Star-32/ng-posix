#include "idt.h"
#include "kernel.h"

static const char *exception_names[] = {
    "Division By Zero",                                     /* 0 */
    "Debug",                                                /* 1 */
    "Non-Maskable Interrupt",                               /* 2 */
    "Breakpoint",                                           /* 3 */
    "Overflow",                                             /* 4 */
    "Bound Range Exceeded",                                 /* 5 */
    "Invalid Opcode",                                       /* 6 */
    "Device Not Available",                                 /* 7 */
    "Double Fault",                                         /* 8 */
    "Coprocessor Segment Overrun",                          /* 9 */
    "Invalid TSS",                                          /* 10 */
    "Segment Not Present",                                  /* 11 */
    "Stack-Segment Fault",                                  /* 12 */
    "General Protection Fault",                             /* 13 */
    "Page Fault",                                           /* 14 */
    "Reserved",                                             /* 15 */
    "x87 FPU Error",                                        /* 16 */
    "Alignment Check",                                      /* 17 */
    "Machine Check",                                        /* 18 */
    "SIMD Floating-Point",                                  /* 19 */
    "Virtualisation",                                       /* 20 */
    "Reserved", "Reserved", "Reserved", "Reserved",         /* 21-24 */
    "Reserved", "Reserved", "Reserved", "Reserved",         /* 25-28 */
    "Reserved",                                             /* 29 */
    "Security Exception",                                   /* 30 */
    "Reserved",                                             /* 31 */
};

void isr_handler(registers_t *regs) {
    vga_print("\n*** EXCEPTION ***\n");

    if (regs->int_no < 32)
        vga_print(exception_names[regs->int_no]);
    else
        vga_print("Unknown");

    vga_print("  [ISR ");
    vga_print_hex(regs->int_no);
    vga_print("]\n");

    vga_print("EIP: "); vga_print_hex(regs->eip);    vga_print("\n");
    vga_print("ERR: "); vga_print_hex(regs->err_code); vga_print("\n");

    vga_print("Halted.");
    for (;;) __asm__ volatile ("cli; hlt");
}
