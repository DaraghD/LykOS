#include "ps2.h"
#include "arch/x86_64/io.h"
#include "drivers/serial.h"
#include "terminal.h"
#include <graphics/draw.h>
#include <stdbool.h>
#include <stdint.h>
#define SHIFT_DOWN 0x2A
#define SHIFT_UP 0xAA
#define CTRL_DOWN 0x1D
#define CTRL_UP 0x9D

volatile u8 keyboard_buffer[KEYBOARD_BUFFER_SIZE];
volatile u8 keyboard_head = 0;
volatile u8 keyboard_tail = 0;

volatile bool shift = false;
volatile bool ctrl = false;
volatile bool arrow_up = false;
volatile bool arrow_down = false;
volatile bool arrow_left = false;
volatile bool arrow_right = false;

__attribute__((interrupt)) void isr_ps2_keyboard(interrupt_frame *frame) {
  (void)frame;
  char scancode = inb(PS2_DATA_PORT);
  u8 next_head = (keyboard_head + 1) % KEYBOARD_BUFFER_SIZE;
  if (next_head != keyboard_tail) {
    keyboard_buffer[keyboard_head] = scancode;
    keyboard_head = next_head;
  }

  end_of_interrupt();
}

#define EXTENDED 0xE0

void keyboard_process(void) {
  static bool extended = false;
  while (keyboard_tail != keyboard_head) {
    u64 sc = keyboard_buffer[keyboard_tail];
    serial_write_fstring("scancode={uint}\n", sc);
    keyboard_tail = (keyboard_tail + 1) % KEYBOARD_BUFFER_SIZE;

    if (sc == EXTENDED) {
      extended = true;
      continue;
    }

    bool is_release = sc & 0x80;
    sc &= 0x7F; // remove release bit

    switch (sc) {
    case 0x2A:
      shift = !is_release;
      continue; // left shift
    case 0x36:
      shift = !is_release;
      continue; // right shift
    case 0x1D:
      ctrl = !is_release;
      continue; // ctrl
    }

    if (extended) {
      extended = false;

      if (is_release)
        continue;

      switch (sc) {
      case 0x48:
        terminal_process_input(UP_ARROW);
        break;
      case 0x50:
        terminal_process_input(DOWN_ARROW);
        break;
      case 0x4B:
        terminal_process_input(LEFT_ARROW);
        break;
      case 0x4D:
        terminal_process_input(RIGHT_ARROW);
        break;
      default:
        break;
      }
      continue;
    }

    if (is_release)
      continue;

    terminal_process_input(sc);
  }
}
