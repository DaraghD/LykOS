#include "terminal.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include "req.h" //lsp bug leave included
#include "shell.h"
#include <stdint.h>

#define DEFAULT_TERMINAL_SCALE 2
// stores input for commands
static char command_buf[256];
static kstring command_content = KSTRING(command_buf, 256);

static const char *lykos_ascii =
    "       __                  __         ______    ______  \n"
    "      /  |                /  |       /      \\  /      \\ \n"
    "      $$ |       __    __ $$ |   __ /$$$$$$  |/$$$$$$  | \n"
    "      $$ |      /  |  /  |$$ |  /  |$$ |  $$ |$$ \\__$$/ \n"
    "      $$ |      $$ |  $$ |$$ |_/$$/ $$ |  $$ |$$      \\ \n"
    "      $$ |      $$ |  $$ |$$   $$<  $$ |  $$ | $$$$$$  | \n"
    "      $$ |_____ $$ \\__$$ |$$$$$$  \\ $$ \\__$$ |/  \\__$$ | \n"
    "      $$       |$$    $$ |$$ | $$  |$$    $$/ $$    $$/  \n"
    "      $$$$$$$$/  $$$$$$$ |$$/   $$/  $$$$$$/   $$$$$$/   \n"
    "                /  \\__$$ |                               \n"
    "                $$    $$/                                \n"
    "                 $$$$$$/                                 \n";

char scancode_to_ascii[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' ',
};

char scancode_to_ascii_caps[128] = {
    0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
    'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' ',
};
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
  draw_cursor_term();
}

void terminal_newline(void) {
  // y_pos += 8 * m_scale;
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
  char c;
  if (shift)
    c = scancode_to_ascii_caps[sc];
  else
    c = scancode_to_ascii[sc];

  if (c) {
    if (c == 'c' && ctrl) {
      uint32_t temp_color = term_color;
      term_color = 0xFFFFFFFF;
      draw_string_term("^C");
      term_color = temp_color;
      y_pos += 8 * m_scale;
      terminal_newline();
      return;
    }
    if (c == 'l' && ctrl) {
      clear_screen(get_framebuffer(), BLACK);
      y_pos = 0;
      terminal_newline();
      return;
    }
    if (c == ENTER) {
      fill_char(x_pos + 8 * m_scale, y_pos);
      fill_char(x_pos, y_pos);
      shell_execute(&command_content);
      draw_cursor_term();
      return;
    }
    if (c == BACKSPACE) {
      command_content.buf[command_content.len - 1] = '\0';
      bool line_not_empty = command_content.len > 0;
      if (line_not_empty) {

        command_content.len -= 1;
        fill_char(x_pos, y_pos);
        x_pos -= 8 * m_scale;
        fill_char(x_pos, y_pos);
        draw_cursor_term();

        bool go_back_line = x_pos == 0;
        if (go_back_line) {
          x_pos = get_framebuffer()->width;
          y_pos -= 8 * m_scale;
        }
      }
      return;
    }
    serial_write_fstring("Writing char: {char} \n", c);
    append_char(&command_content, c);
    fill_char(x_pos, y_pos);
    draw_char_term(c);
    serial_write_fstring("Command buffer size : {int}\n", command_content.len);
  }
  draw_cursor_term();
}
