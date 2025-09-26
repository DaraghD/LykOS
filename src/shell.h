#include "arch/x86_64/util.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include <stdbool.h>
#include <stdint.h>

static uint32_t debug_y = 500;
static const uint32_t debug_x = 500;

void shell_execute(kstring *line) {
  draw_kstring(line, debug_x, debug_y, BLUE);
  debug_y += 8 * m_scale;

  if (kstrncmp(line, "exit", 4)) {
    qemu_shutdown();
  }
  if (kstrncmp(line, "rainbow", 7)) {
        infinite_rainbow(get_framebuffer());
  }
}
