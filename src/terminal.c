#include "terminal.h"
#include "arch/x86_64/util.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include "req.h"
#include "shell.h"
#include <stdint.h>

#define DEFAULT_TERMINAL_SCALE 2
// stores input for commands
static char command_buf[256];
static kstring command_content = KSTRING(command_buf, 256);

static const char *lykos_ascii =
    " __                  __         ______     ______  \n"
    "/  |                /  |       /      \\  /      \\ \n"
    "$$ |       __    __ $$ |   __ /$$$$$$  |/$$$$$$  | \n"
    "$$ |      /  |  /  |$$ |  /  |$$ |  $$ |$$ \\__$$/ \n"
    "$$ |      $$ |  $$ |$$ |_/$$/ $$ |  $$ |$$      \\ \n"
    "$$ |      $$ |  $$ |$$   $$<  $$ |  $$ | $$$$$$  | \n"
    "$$ |_____ $$ \\__$$ |$$$$$$  \\ $$ \\__$$ |/  \\__$$ | \n"
    "$$       |$$    $$ |$$ | $$  |$$    $$/ $$    $$/  \n"
    "$$$$$$$$/  $$$$$$$ |$$/   $$/  $$$$$$/   $$$$$$/   \n"
    "          /  \\__$$ |                               \n"
    "          $$    $$/                                \n"
    "           $$$$$$/                                 \n";

char scancode_to_ascii[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' ',
};
// TODO: backspace to remove characters will probs need a seperate line buffer
// some rerendering

//  static char line_buf[256];
//  static kstring line_content = KSTRING(command_buf, 256);

uint32_t term_color;

void terminal_init(void) {
  serial_writeln("Starting terminal..");
  set_draw_scale(DEFAULT_TERMINAL_SCALE);

  term_color = 0xFFFFFFFF;
  draw_string_term(lykos_ascii);
  draw_string_term("\nWelcome to LykOS\n\n");

  term_color = RED;
  draw_string_term("StarShell>");

  term_color = GREEN;
}

static void terminal_newline(void) {
  y_pos += 8 * m_scale;
  x_pos = 0;

  memset(command_content.buf, 0, command_content.len);
  command_content.len = 0;

  uint32_t temp_color = term_color;
  term_color = RED;
  draw_string_term("StarShell>");
  term_color = temp_color;
}

#define BACKSPACE '\b'
#define ENTER '\n'
void terminal_process_input(uint8_t sc) {
  char c = scancode_to_ascii[sc];
  if (c) {
    if (c == 'c' && ctrl) {
      draw_string_term("ctrl-C");
      terminal_newline();
      return;
    }
    if (c == ENTER) {
      shell_execute(&command_content);
      terminal_newline();
      return;
    }
    if (c == BACKSPACE) {
      command_content.buf[command_content.len - 1] = '\0';
      if (command_content.len > 0) {
        command_content.len -= 1;
        x_pos -= 8 * m_scale;
        fill_char(x_pos, y_pos);
        if (x_pos == 0) {
          x_pos = get_framebuffer()->width;
          y_pos -= 8 * m_scale;
        }
      }
      return;
    }
    if (c == 'q') {
      qemu_shutdown();
    }
    serial_write_fstring("Writing char: {char} \n", c);
    append_char(&command_content, c);
    draw_char_term(c);
    serial_write_fstring("Command buffer size : {int}\n", command_content.len);
  }
}
