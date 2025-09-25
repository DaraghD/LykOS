#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/pic.h"
#include "drivers/ps2.h"
#include "graphics/draw.h"
#include "klib/kstring.h"
#include "terminal.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <cpuid.h>

static void infinite_rainbow(limine_framebuffer *framebuffer) {
  uint64_t colour;
  uint64_t hue = 0;
  // rainbowww
  for (;;) {
    colour = hsv_to_rgb_int(hue, 255, 255);
    clear_screen(framebuffer, colour);
    hue++;
    if (hue >= 360)
      hue = 0;
  }
}

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}
#define CPUID_FEAT_EDX_APIC (1 << 9)
void debug_graphics() {
  limine_framebuffer *framebuffer = get_framebuffer();
  uint32_t center = framebuffer->width / 2;
  center -= 100;

  set_draw_scale(3);
  draw_string("Hello Mars, from LykOS", center, 0, BLUE);
  char height_buf[128];
  kstring height_str = KSTRING(height_buf, 128);
  APPEND_STRL(&height_str, "HEIGHT: ", framebuffer->height);
  draw_kstring(height_str, center, 24, RED);

  char width_buf[128];
  kstring width_string = KSTRING(width_buf, 128);
  APPEND_STRL(&width_string, "WIDTH: ", framebuffer->width);
  APPEND_STRL(&width_string, " pitch: ", framebuffer->pitch);
  draw_kstring(width_string, center, 48, GREEN);
}

void kmain(void) {
  init_graphics();
  gdt_init();
  idt_init();
  pic_remap();
  asm volatile("sti");
  // infinite_rainbow(framebuffer);
  // debug_graphics();
  terminal_init();
  while (1) {
    keyboard_process();
  }
  hcf();
}
