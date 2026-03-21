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
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    uint64_t vector;
    uint64_t error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} interrupt_frame;

void set_idt_gate(u32 index, u64 handler, u16 selector, u8 flags);
void idt_init(void);

typedef void (*isr_handler_fn)(interrupt_frame *frame);

void register_interrupt(uint8_t vector, isr_handler_fn handler);

// ISRs
void isr_divide_error(interrupt_frame *frame);
void isr_test(interrupt_frame *frame);
void isr_ps2_keyboard(interrupt_frame *frame);
void isr_pit(interrupt_frame *frame);
void isr_gpf(interrupt_frame *frame);
void isr_syscall(interrupt_frame *frame);
void isr_pagefault(interrupt_frame *frame);
