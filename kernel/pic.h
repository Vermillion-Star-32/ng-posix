#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1_CMD    0x20    
#define PIC1_DATA   0x21     
#define PIC2_CMD    0xA0     
#define PIC2_DATA   0xA1 

#define PIC_EOI     0x20

#define ICW1_INIT   0x10
#define ICW1_ICW4   0x01

#define ICW4_8086   0x01

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

#define IRQ_TIMER    0
#define IRQ_KEYBOARD 1
#define IRQ_CASCADE  2
#define IRQ_SERIAL2  3
#define IRQ_SERIAL1  4
#define IRQ_PARALLEL 5
#define IRQ_FLOPPY   6
#define IRQ_SPURIOUS 7
#define IRQ_RTC      8
#define IRQ_MOUSE    12
#define IRQ_FPU      13
#define IRQ_ATA1     14
#define IRQ_ATA2     15

void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);
void pic_eoi(uint8_t irq);
void pic_disable(void);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif
