#include "terminal.h"
#include "arch/x86_64/util.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include "req.h"
#include "shell.h"
#include <stdint.h>

#define DEFAULT_TERMINAL_SCALE 2
// stores input for commands
static char line_buf[256];
static kstring line_content = KSTRING(line_buf, 256);

void terminal_init() {
  serial_writeln("Starting terminal..");
  set_draw_scale(DEFAULT_TERMINAL_SCALE);
  draw_string_term("StarShell>", RED);
}
void terminal_process_input(uint8_t sc) {
  // TODO: check for enter, ctrl-c etc
  char c = scancode_to_ascii[sc];
  if (c) {
    if (c == '\n') {
      // TODO: this will eventually have to accomodate for shell output
      // or shell can move the cursor?
      y_pos += 8 * m_scale;
      x_pos = 0;

      shell_execute(&line_content);

      memset(line_content.buf, 0, line_content.len);
      line_content.len = 0;

      draw_string_term("StarShell>", RED);
      return;
    }
    if (c == 'q') {
      qemu_shutdown();
    }
    serial_write_char(c);
    append_char(&line_content, c);
    draw_char_term(c, GREEN);
  }
  // }
}
