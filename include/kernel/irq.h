#ifndef IRQ_H
#define IRQ_H

#include <kernel/idt.h>

typedef void (*irq_handler_t)(registers_t *regs);

void irq_init(void);
void irq_register(int irq, irq_handler_t handler);
void irq_handler(registers_t *regs);

#endif
