#pragma once
#include "arch/x86_64/idt.h"
#include <stdbool.h>
#include <stdint.h>

#define ACK 0xFA
#define RESEND 0xFE
#define PS2_DATA_PORT 0x60

#define KEYBOARD_BUFFER_SIZE 16

#define UP_ARROW 301
#define DOWN_ARROW 302
#define LEFT_ARROW 303
#define RIGHT_ARROW 304

extern volatile bool shift;
extern volatile bool ctrl;
extern volatile bool arrow_up;
extern volatile bool arrow_down;
extern volatile bool arrow_left;
extern volatile bool arrow_right;

// Circular buffer for scancodes
extern volatile u8 keyboard_buffer[KEYBOARD_BUFFER_SIZE];
extern volatile u8 keyboard_head;
extern volatile u8 keyboard_tail;

void keyboard_process(void);
__attribute__((interrupt)) void isr_ps2_keyboard(interrupt_frame *frame);
