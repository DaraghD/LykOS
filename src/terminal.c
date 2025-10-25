#include "terminal.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include "mem/arena.h"
#include "req.h" //lsp bug leave included
#include "shell.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_TERMINAL_SCALE 2
// stores input for commands
static char command_buf[256];
static kstring command_content = KSTRING(command_buf, 256);

// term cursor / drawing position
uint64_t term_xpos = 0;
uint64_t term_ypos = 0;
uint32_t term_color;

static void draw_cursor_term(void);
static void draw_kstring_term(kstring *kstr);
static void clear_current_command(void);

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

typedef struct {
  kstring entries[1000];
  uint32_t count;
  arena arena;
  char *start;
} cmd_history;

cmd_history hist;
static uint32_t hist_position = 0;

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

  hist.arena = arena_init(4096);
  hist.start = hist.arena.beg;
  hist.count = 0;

  shell_init();
}

static void history_add(const kstring *command) {
  if (hist.count == 100) {
    // reset back to start
    hist.arena.beg = hist.start;
  }

  kstring *dst = &hist.entries[hist.count];
  dst->buf = new(&hist.arena, char, command->len);
  memcpy(dst->buf, command->buf, command->len);
  dst->len = command->len;
  dst->cap = command->len + 1;
  dst->error = false;

  hist.count++;
}

void terminal_newline(void) {
  term_xpos = 0;

  memset(command_content.buf, 0, command_content.len);
  command_content.len = 0;

  uint32_t temp_color = term_color;
  term_color = RED;
  draw_string_term("StarShell>");
  term_color = temp_color;
}

static void print_history(void) {
  for (uint32_t i = 0; i < hist.count; i++) {
    kstring *command = &hist.entries[i];
    draw_kstring_term(command);
    draw_string_term("\n");
  }
}

#define BACKSPACE '\b'
#define ENTER '\n'
void terminal_process_input(uint16_t sc) {
  // release keys
  if (sc & 0x80) {
    return;
  }
  char c;
  if (shift)
    c = scancode_to_ascii_caps[sc];
  else
    c = scancode_to_ascii[sc];

  if (c == 'c' && ctrl) {
    uint32_t temp_color = term_color;
    term_color = 0xFFFFFFFF;
    draw_string_term("^C");
    term_color = temp_color;
    term_ypos += 8 * g_scale;
    terminal_newline();
    return;
  }

  if (c == 'h' && ctrl) {
    print_history();
    return;
  }

  if (c == 'l' && ctrl) {
    terminal_clearscreen();
    return;
  }

  if (sc == DOWN_ARROW) {
    if (hist.count == 0)
      return;

    // only move down if we're not already at current input
    if (hist_position > 0)
      hist_position--;

    serial_write_fstring("ARROW DOWN: hist_pos={uint}, count={uint}\n",
                         hist_position, hist.count);

    if (hist_position == 0) {
      // back to empty prompt
      clear_current_command();
      fill_char(term_xpos, term_ypos);
      draw_cursor_term();
      return;
    }

    uint32_t idx = hist.count - hist_position;
    kstring *command = &hist.entries[idx];

    clear_current_command();
    fill_char(term_xpos, term_ypos);
    memset(command_content.buf, 0, command_content.len);
    for (uint16_t i = 0; i < command->len; i++) {
      append_char(&command_content, command->buf[i]);
      draw_char_term(command->buf[i]);
    }
    draw_cursor_term();
    return;
  }
  if (sc == UP_ARROW) {
    if (hist.count == 0)
      return;

    // only move up if we haven't reached the oldest
    if (hist_position < hist.count)
      hist_position++;

    serial_write_fstring("ARROW UP: hist_pos={uint}, count={uint}\n",
                         hist_position, hist.count);

    uint32_t idx = hist.count - hist_position;
    kstring *command = &hist.entries[idx];

    clear_current_command();
    fill_char(term_xpos, term_ypos);
    memset(command_content.buf, 0, command_content.len);
    for (uint16_t i = 0; i < command->len; i++) {
      append_char(&command_content, command->buf[i]);
      draw_char_term(command->buf[i]);
    }
    draw_cursor_term();
    return;
  }

  if (c == ENTER) {
    fill_char(term_xpos + 8 * g_scale, term_ypos);
    fill_char(term_xpos, term_ypos);
    history_add(&command_content);
    shell_execute(&command_content);
    draw_cursor_term();
    return;
  }

  if (c == BACKSPACE) {
    command_content.buf[command_content.len - 1] = '\0';
    bool line_not_empty = command_content.len > 0;
    if (line_not_empty) {

      command_content.len -= 1;
      fill_char(term_xpos, term_ypos);
      term_xpos -= 8 * g_scale;
      fill_char(term_xpos, term_ypos);
      draw_cursor_term();

      bool go_back_line = term_xpos == 0;
      if (go_back_line) {
        term_xpos = get_framebuffer()->width;
        term_ypos -= 8 * g_scale;
      }
    }
    return;
  }

  serial_write_fstring("Writing char: {char} \n", c);
  append_char(&command_content, c);
  fill_char(term_xpos, term_ypos);
  draw_char_term(c);
  serial_write_fstring("Command buffer size : {int}\n", command_content.len);
  draw_cursor_term();
}

// drawing

void terminal_fstring(const char *format, ...) {
  va_list args;
  va_start(args, format);

  write_fstring(TERMINAL, format, args);
  va_end(args);
}

void terminal_clearscreen(void) {
  clear_screen(get_framebuffer(), BLACK);
  term_ypos = 0;
  terminal_newline();
  draw_cursor_term();
}

void draw_char_term(char c) {
  if (c == '\n') {
    term_ypos += 8 * g_scale;
    term_xpos = 0;
  }
  draw_char(c, term_xpos, term_ypos, term_color);
  // advance the cursor
  term_xpos += 8 * g_scale;
  // if next position is greater than the width
  // takes into account how big the START of the next character will be
  if (term_xpos > (get_framebuffer()->width) - (7 * g_scale)) {
    term_ypos += 8 * g_scale;
    term_xpos = 0;
  }
}

static void draw_cursor_term(void) {
  // uint32_t old_color = term_color;
  // term_color = WHITE;
  // draw_char_term('_');
  // term_color = old_color;
  // Instead of using draw_char_term which would advance the cursor
  // automatically Use draw_char which doesnt modify xpos/ypos
  draw_char('_', term_xpos, term_ypos, WHITE);
}

static void draw_kstring_term(kstring *kstr) {
  for (int i = 0; i < kstr->len; i++) {
    if (kstr->buf[i] == '\n') {
      term_xpos = 0;
      term_ypos += 8 * g_scale;
    } else {
      draw_char_term(kstr->buf[i]);
    }
  }
}

void draw_string_term(const char *str) {
  while (*str) {
    if (*str == '\n') {
      term_xpos = 0;
      term_ypos += 8 * g_scale;
    } else {
      draw_char_term(*str);
    }
    str++;
  }
}

static void clear_current_command(void) {
  bool line_not_empty = command_content.len > 0;
  while (line_not_empty) {
    command_content.len -= 1;
    fill_char(term_xpos, term_ypos);
    term_xpos -= 8 * g_scale;
    fill_char(term_xpos, term_ypos);
    draw_cursor_term();

    bool go_back_line = term_xpos == 0;
    if (go_back_line) {
      term_xpos = get_framebuffer()->width;
      term_ypos -= 8 * g_scale;
    }
    line_not_empty = command_content.len > 0;
  }
  memset(command_content.buf, 0, command_content.len);
}
