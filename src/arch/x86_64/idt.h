#pragma once
#include "req.h"
#include <stdint.h>

typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) IDTPointer;

typedef struct {
    u16 offset_low;
    u16 selector;
    u8  ist;
    u8  type_attr;
    u16 offset_mid;
    u32 offset_high;
    u32 zero;
} __attribute__((packed)) IDTEntry;

typedef struct {
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} interrupt_frame;

void set_idt_gate(u32 index, u64 handler, u16 selector, u8 flags);
void idt_init(void);
void idt_remap_init(void);

// ISRs
// Using GCC interrupt attribute
__attribute__((interrupt))
void isr_divide_error(interrupt_frame *frame);
__attribute__((interrupt))
void test_isr(interrupt_frame *frame);
__attribute__((interrupt)) void isr_keyboard(interrupt_frame *frame);
