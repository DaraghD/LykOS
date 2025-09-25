#include "ps2.h"
#include "arch/x86_64/io.h"
#include "terminal.h"
#include <stdbool.h>

volatile uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
volatile uint8_t keyboard_head = 0;
volatile uint8_t keyboard_tail = 0;

static bool shift = false;
static bool ctrl = false;

__attribute__((interrupt)) void isr_ps2_keyboard(interrupt_frame *frame) {
  (void)frame;
  char scancode = inb(PS2_DATA_PORT);
  uint8_t next_head = (keyboard_head + 1) % KEYBOARD_BUFFER_SIZE;
  if (next_head != keyboard_tail) {
    keyboard_buffer[keyboard_head] = scancode;
    keyboard_head = next_head;
  }

  outb(0x20, 0x20);
}

void keyboard_process(void) {
  while (keyboard_tail != keyboard_head) {
    uint8_t sc = keyboard_buffer[keyboard_tail];
    keyboard_tail = (keyboard_tail + 1) % KEYBOARD_BUFFER_SIZE;
    switch (sc) {
    case 0x2A:
      shift = true;
      return; // Shift down
    case 0xAA:
      shift = false;
      return; // Shift up
    case 0x1D:
      ctrl = true;
      return; // Ctrl down
    case 0x9D:
      ctrl = false;
      return; // Ctrl up
    }

    char c = scancode_to_ascii[sc];
    if (!c)
      return;
    // if (ctrl &&
    terminal_process_input(sc);
  }
}
