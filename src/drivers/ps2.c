#include "ps2.h"
#include "arch/x86_64/io.h"
#include "terminal.h"
#include <graphics/draw.h>
#include <stdbool.h>

#define SHIFT_DOWN 0x2A
#define SHIFT_UP 0xAA
#define CTRL_DOWN 0x1D
#define CTRL_UP 0x9D

volatile uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
volatile uint8_t keyboard_head = 0;
volatile uint8_t keyboard_tail = 0;

volatile bool shift = false;
volatile bool ctrl = false;

__attribute__((interrupt)) void isr_ps2_keyboard(interrupt_frame *frame) {
  (void)frame;
  char scancode = inb(PS2_DATA_PORT);
  uint8_t next_head = (keyboard_head + 1) % KEYBOARD_BUFFER_SIZE;
  if (next_head != keyboard_tail) {
    keyboard_buffer[keyboard_head] = scancode;
    keyboard_head = next_head;
  }

  end_of_interrupt();
}

void keyboard_process(void) {
  while (keyboard_tail != keyboard_head) {
    uint8_t sc = keyboard_buffer[keyboard_tail];
    keyboard_tail = (keyboard_tail + 1) % KEYBOARD_BUFFER_SIZE;
    switch (sc) {
    case SHIFT_DOWN:
      shift = true;
      return;
    case SHIFT_UP:
      shift = false;
      return;
    case CTRL_DOWN:
      ctrl = true;
      return;
    case CTRL_UP:
      ctrl = false;
      return;
    }

    // for now ignore other key releases (high bit set)
    if (sc & 0x80)
      return;

    char c = scancode_to_ascii[sc];
    if (!c)
      return;
    terminal_process_input(sc);
  }
}
