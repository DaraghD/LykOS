#pragma once
#include "arch/x86_64/idt.h"
#include <stdbool.h>
#include <stdint.h>

#define ACK 0xFA
#define RESEND 0xFE
#define PS2_DATA_PORT 0x60

#define KEYBOARD_BUFFER_SIZE 16

extern volatile bool shift;
extern volatile bool ctrl;

// Circular buffer for scancodes
extern volatile uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
extern volatile uint8_t keyboard_head;
extern volatile uint8_t keyboard_tail;

void keyboard_process(void);
__attribute__((interrupt)) void isr_ps2_keyboard(interrupt_frame *frame);
