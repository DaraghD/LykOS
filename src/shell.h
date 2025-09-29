#include "arch/x86_64/util.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include "mem/mem.h"
#include "terminal.h"
#include <stdbool.h>
#include <stdint.h>

static uint32_t debug_y = 500;
static const uint32_t debug_x = 500;

void shell_execute(kstring *line) {
  y_pos += 8 * m_scale;
  x_pos = 0;

  if (kstrncmp(line, "exit", 4))
    qemu_shutdown();

  else if (kstrncmp(line, "rainbow", 7))
    infinite_rainbow(get_framebuffer());

  else if (kstrncmp(line, "red", 3))
    term_color = RED;

  else if (kstrncmp(line, "blue", 4))
    term_color = BLUE;

  else if (kstrncmp(line, "green", 5))
    term_color = GREEN;

  else if (kstrncmp(line, "mem", 3))
    print_memmap();

  else if (kstrncmp(line, "paging", 6))
    print_paging();

  else if (kstrncmp(line, "clear", 5)){
    clear_screen(get_framebuffer(), BLACK);
    y_pos = 0;
  }

  else if (line->len >= 1) {
    draw_string_term("Unknown command\n");
  }

  terminal_newline();
}
