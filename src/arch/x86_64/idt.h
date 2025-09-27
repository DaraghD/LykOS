#pragma once
#include <stdint.h>

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) IDTPointer;

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) IDTEntry;

typedef struct {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} interrupt_frame;

void set_idt_gate(uint32_t index, uint64_t handler, uint16_t selector, uint8_t flags);
void idt_init(void);
void idt_remap_init(void);

// ISRs
// Using GCC interrupt attribute
__attribute__((interrupt))
void isr_divide_error(interrupt_frame *frame);
__attribute__((interrupt))
void test_isr(interrupt_frame *frame);
__attribute__((interrupt)) void isr_keyboard(interrupt_frame *frame);
